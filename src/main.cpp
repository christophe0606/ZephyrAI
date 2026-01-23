/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream demo
 * Title:        main.cpp
 * Description:  Entry point for the CMSIS Stream demo application
 *
 * --------------------------------------------------------------------
 *
 * Copyright (C) 2026 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
#include "appc_params.h"

#include "scheduler_appa.h"
#include "scheduler_appb.h"
#include "scheduler_appc.h"

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

#define NB_APPS 3
// Start with KWS demo
// 0 : KWS
// 1 : Spectrogram
// 2 : To experiment with camera support
static int currentNetwork = 0;

#define SWITCH_EVENT (1 << 0)

/**
 * @brief Array of stream execution contexts, one per network.
 */
static stream_execution_context_t contexts[NB_APPS];

/**
 * @brief Parameters for appa and appb networks.
 * By convention, each parameter structs starts with a hardwareParams member
 * named 'hw_' that holds hardware connection parameters.
 */
static hardwareParams *params[NB_APPS];


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
		currentNetwork = (currentNetwork + 1) % NB_APPS;
		LOG_DBG("Switching to network %d\n", currentNetwork);
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
			// If the node implements the context switch interface, pause it
			if ((cnode->obj != nullptr) && (cnode->context_switch_intf != nullptr))
			{
				cnode->context_switch_intf->pause(cnode->obj);
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
			// If the node implements the context switching interface, resume it
			if ((cnode->obj != nullptr) && (cnode->context_switch_intf != nullptr))
			{
				cnode->context_switch_intf->resume(cnode->obj);
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

static void* get_appc_node(int32_t nodeID)
{
	return static_cast<void *>(get_scheduler_appc_node(nodeID));
}

int main(void)
{   
	int err;
	EventQueue *queue_app[NB_APPS];


	LOG_DBG("Starting main\n");

	/*
	Configure hardware audio source and display
	Initialize memory slab for audio buffers

	Instead of hardcoding initialization of HW peripheral here,
	one could extend the ContextSwitch interface to query what
	is the need of a node.
	Then, we could ask all nodes in each graph what they need.
	And we would only initialize the HW peripherals needed by the nodes
	in any graph.
	
	*/

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

    /** 
	 * @brief Network parameter initialization
	 * Most settings could come from a YAML file. The YAML
	 * file could be used to generate the appa_params.c and
	 * appb_params.c files.
	 * 
	 * The parameters related to hardware connections are set here 
	 * in main.cpp. They are common to all graphs so a generic loop
	 * can be used. The convention has been imposed that each
	 * prameter structure for each graph starts with a hardwareParams
	 * member named 'hw_'.
	 * 
	 * The TensorFlow Lite model pointer and size are also set here
	 * in main.cpp for the appa graph. 
	 * It would probaby be better to put this in a YAML file too
	 * (name of variable containing the model pointer and size).
	 * Or have another mechanism to select the model from yaml.
	 * Here we have to hardcode the initialization of the node.
	 * So it is not generic. If the network contains multiple TFLite
	 * nodes with different models, this approach would not work.
	 * We would need to change the initialization.
	 * It would be better if the initialization in this
	 * file could work with any network and don't have to be changed.
	 * If is possible but require to define some conventions
	 */
    
	/*
	
	Init settings for appa scheduler
	
	*/
    appaParams.kws.modelAddr = (uint8_t *)GetModelPointer();
	appaParams.kws.modelSize = GetModelLen();
	params[0] = reinterpret_cast<hardwareParams *>(&appaParams);

	/*
	
	Init settings for appb scheduler
	
	*/
	params[1] = reinterpret_cast<hardwareParams *>(&appbParams);

	/*
	
	Init settings for appc scheduler
	
	*/
	params[2] = reinterpret_cast<hardwareParams *>(&appcParams);

	/**
	 * @brief Populate hardwareParams for each network
	 * by setting the i2s_mic and mem_slab members.
	 */
	for(int network=0; network < NB_APPS; network++) 
	{
		params[network]->i2s_mic = i2s_mic;
	    params[network]->mem_slab = mem_slab;
	}

	err = stream_init_memory();
	if (err != 0)
	{
		LOG_ERR("Error initializing stream\n");
		goto error;
	}

	/* Event queue init */
	for(int network=0; network < NB_APPS; network++) 
	{
		queue_app[network] = stream_new_event_queue();

		if (queue_app[network] == nullptr) {
			LOG_ERR("Can't create CMSIS Stream Event Queue for network %d\n",network);
			goto error;
		}
	}
	
	// Init nodes
	err = init_scheduler_appa(queue_app[0],&appaParams);
	if (err != CG_SUCCESS) {
		LOG_ERR("Error: Failure during scheduler initialization for appa.\n");
		goto error;
	}

	err = init_scheduler_appb(queue_app[1],&appbParams);
	if (err != CG_SUCCESS) {
		LOG_ERR("Error: Failure during scheduler initialization for appb.\n");
		goto error;
	}

	err = init_scheduler_appc(queue_app[2],&appcParams);
	if (err != CG_SUCCESS) {
		LOG_ERR("Error: Failure during scheduler initialization for appc.\n");
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
		.evtQueue = queue_app[0],
		.nb_identified_nodes = STREAM_APPA_NB_IDENTIFIED_NODES,
		.scheduler_length = STREAM_APPA_SCHED_LEN
	};

	contexts[1] = {
		.dataflow_scheduler = scheduler_appb,
		.reset_fifos = reset_fifos_scheduler_appb,
		.pause_all_nodes = pause_scheduler_app,
		.resume_all_nodes = resume_scheduler_app,
		.get_node_by_id = get_appb_node,
		.evtQueue = queue_app[1],
		.nb_identified_nodes = STREAM_APPB_NB_IDENTIFIED_NODES,
		.scheduler_length = STREAM_APPB_SCHED_LEN
	};

	
	contexts[2] = {
		.dataflow_scheduler = scheduler_appc,
		.reset_fifos = reset_fifos_scheduler_appc,
		.pause_all_nodes = pause_scheduler_app,
		.resume_all_nodes = resume_scheduler_app,
		.get_node_by_id = get_appc_node,
		.evtQueue = queue_app[2],
		.nb_identified_nodes = STREAM_APPC_NB_IDENTIFIED_NODES,
		.scheduler_length = STREAM_APPC_SCHED_LEN
	};
	

	LOG_INF("Try to start first network");

	
	stream_start_threads(&contexts[currentNetwork]);

	stream_wait_for_threads_end();

	free_scheduler_appa();
	free_scheduler_appb();
	free_scheduler_appc();
	
	for(int network=0; network < NB_APPS; network++) 
	{
		delete queue_app[network];
	}

	stream_free_memory();

	k_sleep(K_FOREVER);

error:
    LOG_ERR("Fatal error in main, stopping execution\n");
    k_sleep(K_FOREVER);
	return 0;

}
