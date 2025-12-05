#include <stdio.h>
#include <vector>

#include "dsp/basic_math_functions.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(stream);

#include "stream_types.hpp"
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "cstream_node.h"

#include "cg_queue.hpp"

extern "C" {
#include "stream.h"
#include "scheduler.h"

}



struct k_mem_slab cg_eventPool;
struct k_mem_slab cg_bufPool;
struct k_mem_slab cg_mutexPool;

// Event to the event thread
struct k_event cg_eventEvent;

// Event to the interrupt thread
struct k_event cg_interruptEvent;

// Event to the stream thread
struct k_event cg_streamEvent;


static void *event_pool_buffer;
static void *buf_pool_buffer;
static void *mutex_pool_buffer;


static k_tid_t tid_stream = nullptr;
static k_tid_t cg_eventThread = nullptr;
static k_tid_t tid_interrupts = nullptr;

static struct k_thread stream_thread;
static struct k_thread event_thread;
static struct k_thread interrupt_thread;


#define NB_MAX_EVENTS 20
#define NB_MAX_BUFS 20

#define AUDIO_THREAD_PRIORITY (0)
#define NORMAL_PRIORITY 5

static K_THREAD_STACK_DEFINE(interrupt_thread_stack, 1024);
static K_THREAD_STACK_DEFINE(event_thread_stack, 1024);
static K_THREAD_STACK_DEFINE(stream_thread_stack, 4096);

#define SRAM0_HEAP_SIZE 200000

__aligned(8)
__attribute__((section(".alif_sram0")))
static uint8_t sram0_heap_area[SRAM0_HEAP_SIZE];

static struct k_heap sram0_heap;

using namespace arm_cmsis_stream;

// Translate interrupt events into CMSIS Stream events
void interrupt_thread_function(void *, void *, void *)
{
    LOG_INF("Started interrupt thread\n");
    // There no interrupt event (yet) to transmit to the
    // CMSIS Stream graph. So this thread is empty for now.
}

void event_thread_function(void *, void *, void *)
{
    LOG_INF("Started event thread\n");

    

    arm_cmsis_stream::EventQueue::cg_eventQueue->execute();

    // Delete the event queue when done

    delete arm_cmsis_stream::EventQueue::cg_eventQueue;
    arm_cmsis_stream::EventQueue::cg_eventQueue = nullptr;


}

void stream_thread_function(void *, void *, void *)
{
    uint32_t nb_iter;
    int error;
    LOG_INF("Stream thread started\n");

    // Init nodes and starts audio stream
    error = init_scheduler();
    if (error != CG_SUCCESS)
    {
        LOG_ERR("Error: Failure during scheduler initialization.\n");
        goto err_main;
    }
    

    LOG_INF("Starting scheduler\n");
    nb_iter = scheduler(&error);
    if (error != 0)
    {
        LOG_ERR("Scheduler error %d\n", error);
    }
    LOG_INF("Scheduler done after %d iterations\n", nb_iter);

err_main:
    LOG_INF("End stream thread\n");
    free_scheduler();

}

int init_stream()
{
    /* Init sram0 heap */
    k_heap_init(&sram0_heap, sram0_heap_area, SRAM0_HEAP_SIZE);
    
    /* Init memory slabs */
    event_pool_buffer = nullptr;
    buf_pool_buffer = nullptr;
    mutex_pool_buffer = nullptr;

    event_pool_buffer = k_heap_alloc(&sram0_heap, NB_MAX_EVENTS * (sizeof(ListValue) + 16),K_NO_WAIT);
    int err = k_mem_slab_init(&cg_eventPool, event_pool_buffer, sizeof(ListValue) + 16,NB_MAX_EVENTS);
    if (err != 0)
    {
        LOG_ERR("Failed to init event pool slab\n");
        return(err);
    }

    buf_pool_buffer = k_heap_alloc(&sram0_heap, NB_MAX_BUFS * (sizeof(Tensor<double>) + 16),K_NO_WAIT);
    err = k_mem_slab_init(&cg_bufPool, buf_pool_buffer, sizeof(Tensor<double>) + 16,NB_MAX_BUFS);
    if (err != 0)
    {
        LOG_ERR("Failed to init buf pool slab\n");
        return(err);
    }

    mutex_pool_buffer = k_heap_alloc(&sram0_heap, NB_MAX_BUFS * (sizeof(CG_MUTEX) + 16),K_NO_WAIT);
    err = k_mem_slab_init(&cg_mutexPool, mutex_pool_buffer, sizeof(CG_MUTEX) + 16,NB_MAX_BUFS);
    if (err != 0)
    {
        LOG_ERR("Failed to init mutex pool slab\n");
        return(err);
    }

    /* Init events */
    k_event_init(&cg_eventEvent);
    k_event_init(&cg_interruptEvent);
    k_event_init(&cg_streamEvent);

    /* Event queue init */
    arm_cmsis_stream::EventQueue::cg_eventQueue = new (std::nothrow) MyQueue(6, 5, 4);
    if (arm_cmsis_stream::EventQueue::cg_eventQueue == nullptr)
    {
        LOG_ERR("Can't create CMSIS Event Queue\n");
        return(ENOMEM);
    }


    /* Thread inits */
    tid_interrupts = k_thread_create(&interrupt_thread, interrupt_thread_stack,
                                 K_THREAD_STACK_SIZEOF(interrupt_thread_stack),
                                 interrupt_thread_function,
                                 NULL, NULL, NULL,
                                 NORMAL_PRIORITY, 0, K_NO_WAIT);

    k_thread_name_set(&interrupt_thread, "interrupt_to_evt");


    cg_eventThread = k_thread_create(&event_thread, event_thread_stack,
                                 K_THREAD_STACK_SIZEOF(event_thread_stack),
                                 event_thread_function,
                                 NULL, NULL, NULL,
                                 NORMAL_PRIORITY, K_FP_REGS, K_NO_WAIT);

    k_thread_name_set(&event_thread, "event_thread");


    tid_stream = k_thread_create(&stream_thread, stream_thread_stack,
                                 K_THREAD_STACK_SIZEOF(stream_thread_stack),
                                 stream_thread_function,
                                 NULL, NULL, NULL,
                                 AUDIO_THREAD_PRIORITY, K_FP_REGS, K_NO_WAIT);

    k_thread_name_set(&stream_thread, "stream_thread");

    printf("Stream initialized\n");

    return(0);
}

void deinit_stream()
{
   
}