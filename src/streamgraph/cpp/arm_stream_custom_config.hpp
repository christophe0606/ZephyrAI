#pragma once 

//#include "arm_mve.h"
#include "arm_math_types.h"

extern "C"
{
#include <zephyr/kernel.h>


extern struct k_event cg_streamEvent;

}

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(streamsched);

struct cf32 {
    float real;
    float imag;
};

struct sf32 {
    float left;
    float right;
};

struct cq15 {
    int16_t real;
    int16_t imag;
};

struct sq15 {
    int16_t left;
    int16_t right;
};



#include "rtos_events.hpp"



#define CG_BEFORE_SCHEDULE \
  uint32_t errorFlags = 0;


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

// To keep a C API for the scheduler we pass a void * for the
// EventQueue and it needs to be casted to the right type
// (We could also use the CMSIS Stream option to have a C++ API)
#define CG_BEFORE_FIFO_INIT \
  EventQueue *evtQueue = reinterpret_cast<EventQueue *>(evtQueue_);

#include "cmsisstream_zephyr_config.hpp"