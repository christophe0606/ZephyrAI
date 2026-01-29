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
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>


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

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW_NODE	DT_ALIAS(sw0)
#if DT_NODE_HAS_STATUS_OKAY(SW_NODE)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW_NODE, gpios,
							      {0});
static struct gpio_callback button_cb_data;
#endif

#if DT_NODE_EXISTS(DT_CHOSEN(zephyr_touch))
static const struct device *const touch_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_touch));

#endif

// Event to the interrupt thread
struct k_event cg_interruptEvent;
static k_tid_t tid_interrupts = NULL;
static struct k_thread interrupt_thread;

#define INTERRUPT_THREAD_PRIORITY 0

static K_THREAD_STACK_DEFINE(interrupt_thread_stack, 4096);

using namespace arm_cmsis_stream;

// Number of applications/networks available in this demo
#define NB_APPS 3
// 0 : KWS
// 1 : Spectrogram
// 2 : To experiment with camera support
static int currentNetwork = 2;

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

#if DT_NODE_HAS_STATUS_OKAY(SW_NODE)
void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
    uint32_t old = k_event_post(&cg_interruptEvent, SWITCH_EVENT);
	LOG_DBG("Posted SWITCH_EVENT, old events=0x%08x\n",old);
	
}
#endif 

#if DT_NODE_EXISTS(DT_CHOSEN(zephyr_touch))
static int64_t last_sync_ms=0;

static void touch_event_callback(struct input_event *evt, void *user_data)
{
	if (evt->sync) {
		int64_t now = k_uptime_get();
		if (now - last_sync_ms > CONFIG_TOUCH_SCREEN_DELAY) {
            uint32_t old = k_event_post(&cg_interruptEvent, SWITCH_EVENT);
	        LOG_DBG("Posted SWITCH_EVENT, old events=0x%08x\n",old);
			last_sync_ms = now;
        }
		
	}
}
INPUT_CALLBACK_DEFINE(touch_dev, touch_event_callback, NULL);
#endif

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

/*

 Pause and resume all nodes in a given graph.
 Those are not defined in the CMSIS STream Zephyr module because the
 CStreamNode interface can be extended by the application and support new
 interfaces.

 To work with the CMSIS Stream Zephyr module, two interfaces have been defined
 in cstream_node.h and the context_switch_intf is used in the 
 pause / resume functions.

 Those functions are called by the CMSIS Stream Zephyr module which does not
 have any visibility on the application specific CStreamNode structure.

*/
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

/*

The get_scheduler_node functions cannot be used directly by the
CMSIS Stream Zephyr module necause they return an application dependent
CStreamNode* pointer.
So we need to wrap them to return a void* pointer.

*/
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

#if DT_NODE_HAS_STATUS_OKAY(SW_NODE)
static int config_button()
{
	if (!gpio_is_ready_dt(&button)) {
		printk("Error: button device %s is not ready\n",
		       button.port->name);
		return -1;
	}

	int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button.port->name, button.pin);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button.port->name, button.pin);
		return ret;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	return(0);
}
#endif 

#if DT_NODE_EXISTS(DT_CHOSEN(zephyr_touch))
static int config_touch()
{
	if (!device_is_ready(touch_dev)) {
		LOG_ERR("Device %s not found. Aborting sample.", touch_dev->name);
		return -1;
	}


	return(0);
}
#endif

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

#if DT_NODE_HAS_STATUS_OKAY(SW_NODE)
  err = config_button();
  if (err != 0) {
	  LOG_ERR("Error configuring button\n");
	  goto error;
  }
#endif 

#if DT_NODE_EXISTS(DT_CHOSEN(zephyr_touch))
  err = config_touch();
  if (err != 0) {
	  LOG_ERR("Error configuring touch\n");
	  goto error;
  }
#endif

#if defined(CONFIG_DISPLAY)
	err = init_display();
	if (err != 0) {
		LOG_ERR("Error initializing display\n");
		goto error;
	}
#endif



#if defined(CONFIG_MODEL_IN_EXT_FLASH)
   err = validate_network_description("c147eeaf87c900ff230950424d07ca07");
   if (err) {
	   LOG_ERR("Invalid network description in external flash\n");
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


	/*

	resume is called (like in a context switch) to allow
	the nodes to publish events before starting if they need to.
	event queue accepts events when resume is called.

	We cannot do it in init function of the node since in init we 
	do not know if the graph is going to be run or if it is paused.

	If there is a need to distinguish start from resume then 
	it has to be done in each node with a state variable.
	
	*/
	resume_scheduler_app(&contexts[currentNetwork]);
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
