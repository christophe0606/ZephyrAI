/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include "cg_enums.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(mainapp);

#include "cstream_node.h"
#include "scheduler_grapha.h"
#include "runtime_init.h"


// Event to the interrupt thread
struct k_event cg_interruptEvent;
static k_tid_t tid_interrupts = NULL;
static struct k_thread interrupt_thread;

struct k_event cg_streamEvent;

static K_THREAD_STACK_DEFINE(interrupt_thread_stack, 4096);

// Translate interrupt events into CMSIS Stream events
void interrupt_thread_function(void *, void *, void *)
{
	LOG_INF("Started interrupt thread\n");
#if defined(STREAMVIDEO_ID)
	// There no interrupt event (yet) to transmit to the
	// CMSIS Stream graph. So this thread is empty for now.
	CStreamNode *cvideoNode = get_scheduler_node(STREAMVIDEO_ID);
	if (cvideoNode == nullptr) {
		LOG_ERR("Can't get cvideo node\n");
		return;
	}

	StreamNode *videoNode = static_cast<StreamNode *>(cvideoNode->obj);
	if (videoNode == nullptr) {
		LOG_ERR("Can't get video node\n");
		return;
	}

	for (;;) {
		k_sleep(K_MSEC(100));
        Event evt{kDo, kHighPriority};
		arm_cmsis_stream::EventQueue::cg_eventQueue->push(LocalDestination{videoNode, 0},std::move(evt));
	}
#endif
}

int main(void)
{   
	LOG_DBG("Starting main\n");

	k_event_init(&cg_streamEvent);


	int err = init_stream_memory();
	if (err != 0)
	{
		LOG_ERR("Error initializing stream\n");
		return -1;
	}

	/* Event queue init */
	void *queue_grapha = new_event_queue();

	if (queue_grapha == nullptr) {
		LOG_ERR("Can't create CMSIS Stream Event Queue\n");
		return (ENOMEM);
	}
	
	

	// Init nodes
	err = init_scheduler_grapha(nullptr);
	if (err != CG_SUCCESS) {
		LOG_ERR("Error: Failure during scheduler initialization for grapha.\n");
		return (ENOMEM);
	}


	k_event_init(&cg_interruptEvent);

	/* Thread inits */
	tid_interrupts = k_thread_create(&interrupt_thread, interrupt_thread_stack,
					 K_THREAD_STACK_SIZEOF(interrupt_thread_stack),
					 interrupt_thread_function, NULL, NULL, NULL,
					 5, 0, K_NO_WAIT);

	k_thread_name_set(&interrupt_thread, "interrupt_to_evt");

	start_stream_threads(&scheduler_grapha,nullptr);
	
	wait_for_stream_thread_end();

	free_scheduler_grapha(nullptr);

	free_stream_memory();

	return 0;

}
