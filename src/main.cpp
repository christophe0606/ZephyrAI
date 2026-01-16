/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdio>
#include <cstdlib>

#include "cg_enums.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/i2s.h>

LOG_MODULE_REGISTER(streamapps,CONFIG_STREAMAPPS_LOG_LEVEL);

#include "cstream_node.h"
#include "appa_params.h"
#include "appb_params.h"

#include "scheduler_appa.h"
#include "scheduler_appb.h"

#include "EventQueue.hpp"
#include "StreamNode.hpp"

#include "stream_runtime_init.hpp"

#include "rtos_events.hpp"

extern "C"
{
	#include "network.h"
}

#include "init_drv_src.hpp"

// Event to the interrupt thread
struct k_event cg_interruptEvent;
static k_tid_t tid_interrupts = NULL;
static struct k_thread interrupt_thread;

#define INTERRUPT_THREAD_PRIORITY 0

static K_THREAD_STACK_DEFINE(interrupt_thread_stack, 4096);

using namespace arm_cmsis_stream;

#define NB_NETWORKS 2
static int currentNetwork = 0;

#define SWITCH_EVENT (1 << 0)

static stream_execution_context_t contexts[NB_NETWORKS];


static int cmd_switch(const struct shell *shell,
                     size_t argc, char **argv)
{
	uint32_t old = k_event_post(&cg_interruptEvent, SWITCH_EVENT);
	LOG_DBG("Posted SWITCH_EVENT, old events=0x%08x\n",old);
	
	return 0;
}

SHELL_CMD_REGISTER(switch, NULL,
                   "Switch between networks",
                   cmd_switch);

// Translate interrupt events into CMSIS Stream events
void interrupt_thread_function(void *, void *, void *)
{
   LOG_INF("Started interrupt thread\n");

   for(;;)
   {
	uint32_t res = k_event_wait(&cg_interruptEvent, SWITCH_EVENT, false, K_FOREVER);
	if ((res & SWITCH_EVENT) != 0)
	{
		k_event_clear(&cg_interruptEvent, SWITCH_EVENT);
		LOG_DBG("Received Switching network event\n");
		currentNetwork = (currentNetwork + 1) % NB_NETWORKS;
		stream_pause_current_scheduler();
	    stream_resume_scheduler(&contexts[currentNetwork]);
		LOG_DBG("Context switch done\n");
	}
   }
   LOG_INF("Interrupt thread ended\n");

}

static void pause_scheduler_app(const stream_execution_context_t* context)
{
	for(int32_t nodeid=0 ; nodeid < (int32_t)context->nb_identified_nodes ; nodeid++) {
		CStreamNode *cnode = static_cast<CStreamNode *>(context->get_node_by_id(nodeid));
		if (cnode != nullptr) 
		{
			// If the node implements the hardware connection interface, pause it
			if ((cnode->obj != nullptr) && (cnode->hw_conn_intf != nullptr))
			{
				cnode->hw_conn_intf->pause(cnode->obj);
			}
		}
	}
}

static void resume_scheduler_app(const stream_execution_context_t* context)
{
	for(int32_t nodeid=0 ; nodeid < (int32_t)context->nb_identified_nodes ; nodeid++) {
		CStreamNode *cnode = static_cast<CStreamNode *>(context->get_node_by_id(nodeid));
		if (cnode != nullptr) 
		{
			// If the node implements the hardware connection interface, resume it
			if ((cnode->obj != nullptr) && (cnode->hw_conn_intf != nullptr))
			{
				cnode->hw_conn_intf->resume(cnode->obj);
			}
		}
	}
}

static void* get_appa_node(int32_t nodeID)
{
	return static_cast<void *>(get_scheduler_appa_node(nodeID));
}

static void* get_appb_node(int32_t nodeID)
{
	return static_cast<void *>(get_scheduler_appb_node(nodeID));
}

int main(void)
{   
	LOG_DBG("Starting main\n");

	/*
	Configure hardware audio source and display
	Initialize memory slab for audio buffers

	Instead of hardcoding initialization of HW peripheral here,
	one could extend the HardwareConnection interface to query what
	is the need of a node.
	Then, we could ask all nodes in each graph what they need.
	And we would only initialize the HW peripherals needed by the nodes
	in any graph.
	
	*/

	int err;
	EventQueue *queue_appa;
	EventQueue *queue_appb;
#if defined(CONFIG_I2S)
	k_mem_slab *mem_slab = nullptr;
	const struct device *i2s_mic = init_audio_source(&mem_slab);
	if (i2s_mic == nullptr) {
		LOG_ERR("Error initializing audio source\n");
		goto error;
	}
#else
   k_mem_slab *mem_slab = nullptr;
   const struct device *i2s_mic = nullptr;
#endif

#if defined(CONFIG_DISPLAY)
	err = init_display();
	if (err != 0) {
		LOG_ERR("Error initializing display\n");
		goto error;
	}
#endif
    
	/*
	
	Init settings for appa scheduler
	
	*/
    appaParams.kws.modelAddr = (uint8_t *)GetModelPointer();
	appaParams.kws.modelSize = GetModelLen();
	appaParams.audioSource.i2s_mic = i2s_mic;
	appaParams.audioSource.mem_slab = mem_slab;

	/*
	
	Init settings for appb scheduler
	
	*/
	appbParams.audio.i2s_mic = i2s_mic;
	appbParams.audio.mem_slab = mem_slab;

	err = stream_init_memory();
	if (err != 0)
	{
		LOG_ERR("Error initializing stream\n");
		goto error;
	}

	/* Event queue init */
	queue_appa = stream_new_event_queue();

	if (queue_appa == nullptr) {
		LOG_ERR("Can't create CMSIS Stream Event Queue for appa\n");
		goto error;
	}

	queue_appb = stream_new_event_queue();

	if (queue_appb == nullptr) {
		LOG_ERR("Can't create CMSIS Stream Event Queue for appb\n");
		goto error;
	}
	
	

	// Init nodes
	err = init_scheduler_appa(queue_appa,&appaParams);
	if (err != CG_SUCCESS) {
		LOG_ERR("Error: Failure during scheduler initialization for appa.\n");
		goto error;
	}

	err = init_scheduler_appb(queue_appb,&appbParams);
	if (err != CG_SUCCESS) {
		LOG_ERR("Error: Failure during scheduler initialization for appb.\n");
		goto error;
	}


	k_event_init(&cg_interruptEvent);

	/* Thread inits */
	tid_interrupts = k_thread_create(&interrupt_thread, interrupt_thread_stack,
					 K_THREAD_STACK_SIZEOF(interrupt_thread_stack),
					 interrupt_thread_function, NULL, NULL, NULL,
					 INTERRUPT_THREAD_PRIORITY, K_FP_REGS, K_NO_WAIT);

	k_thread_name_set(&interrupt_thread, "interrupt_to_evt");

	LOG_INF("Initialize contexts");

	contexts[0] = {
		.dataflow_scheduler = scheduler_appa,
		.reset_fifos = reset_fifos_scheduler_appa,
		.pause_all_nodes = pause_scheduler_app,
		.resume_all_nodes = resume_scheduler_app,
		.get_node_by_id = get_appa_node,
		.evtQueue = queue_appa,
		.nb_identified_nodes = STREAM_APPA_NB_IDENTIFIED_NODES
	};

	contexts[1] = {
		.dataflow_scheduler = scheduler_appb,
		.reset_fifos = reset_fifos_scheduler_appb,
		.pause_all_nodes = pause_scheduler_app,
		.resume_all_nodes = resume_scheduler_app,
		.get_node_by_id = get_appb_node,
		.evtQueue = queue_appb,
		.nb_identified_nodes = STREAM_APPB_NB_IDENTIFIED_NODES
	};

	LOG_INF("Try to start first network");

	stream_start_threads(&contexts[currentNetwork]);

	stream_wait_for_threads_end();

	free_scheduler_appa();
	free_scheduler_appb();

	delete queue_appa;
	delete queue_appb;

	stream_free_memory();

	k_sleep(K_FOREVER);

error:
    LOG_ERR("Fatal error in main, stopping execution\n");
    k_sleep(K_FOREVER);
	return 0;

}
