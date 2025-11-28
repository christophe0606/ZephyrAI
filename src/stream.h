#ifndef stream_H
#define stream_H

#include <zephyr/kernel.h>

#ifdef   __cplusplus
extern "C"
{
#endif

extern struct k_mem_slab cg_eventPool;
extern struct k_mem_slab cg_bufPool;
extern struct k_mem_slab cg_mutexPool;
extern struct k_event cg_eventEvent;
extern struct k_event cg_streamEvent;


//extern k_tid_t tid_stream;
//extern k_tid_t cg_eventThread;
//extern k_tid_t tid_interrupts;

int init_stream();
void deinit_stream();

#ifdef   __cplusplus
}
#endif

#endif