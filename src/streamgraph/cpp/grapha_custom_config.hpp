#pragma once 

/**
 * 
 * This file is used to customize the dataflow loop and provide
 * some definition  to the scheduler.cpp
 * Generally this file defines some macros used in the
 * scheduler dataflow loop and some datatypes used in the project.
 * 
 */


extern "C"
{
#include <zephyr/kernel.h>


extern struct k_event cg_streamEvent;

}

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(streamsched);



#include "rtos_events.hpp"



#define CG_BEFORE_SCHEDULE \
  uint32_t errorFlags = 0;


// Example of how to inject code before each node execution in the
// dataflow graph scheduler loop
#define CG_BEFORE_NODE_EXECUTION(ID)                                                                                       \
{                                                                                                                          \
    errorFlags = k_event_wait(&cg_streamEvent,AUDIO_SINK_UNDERFLOW_EVENT | AUDIO_SOURCE_OVERFLOW_EVENT, false, K_NO_WAIT); \
    k_event_clear(&cg_streamEvent, AUDIO_SINK_UNDERFLOW_EVENT | AUDIO_SOURCE_OVERFLOW_EVENT);                                                                \
                                                                                                                           \
    if (errorFlags & AUDIO_SOURCE_OVERFLOW_EVENT)                                                                          \
    {                                                                                                                      \
        cgStaticError = CG_BUFFER_OVERFLOW;                                                                                \
        goto errorHandling;                                                                                                \
    }                                                                                                                      \
}                             


#include "datatypes.hpp"