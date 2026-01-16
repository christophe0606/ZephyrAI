#pragma once

/**
 *
 * This file is used to customize the dataflow loop and provide
 * some definition  to the scheduler.cpp
 * Generally this file defines some macros used in the
 * scheduler dataflow loop and some datatypes used in the project.
 *
 */

#include <zephyr/kernel.h>
#include <cstdlib>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(streamapps,CONFIG_STREAMAPPS_LOG_LEVEL);

#include "rtos_events.hpp"

#include "datatypes.hpp"

#include "appa_params.h"
#include "appb_params.h"

extern struct k_event cg_streamEvent;

#define CG_BEFORE_NODE_EXECUTION(id)                                              \
    {                                                                             \
        uint32_t res =                                                            \
            k_event_wait(&cg_streamEvent, STREAM_PAUSE_EVENT | STREAM_DONE_EVENT, \
                     false, K_NO_WAIT);                                           \
        if ((res & STREAM_DONE_EVENT) != 0)                                       \
        {                                                                         \
            k_event_clear(&cg_streamEvent, STREAM_DONE_EVENT);                    \
            cgStaticError = CG_STOP_SCHEDULER;                                    \
            goto errorHandling;                                                   \
        }                                                                         \
        if ((res & STREAM_PAUSE_EVENT) != 0)                                      \
        {                                                                         \
            k_event_clear(&cg_streamEvent, STREAM_PAUSE_EVENT);                   \
            cgStaticError = CG_PAUSED_SCHEDULER;                                  \
            goto errorHandling;                                                   \
        }                                                                         \
   }

class HardwareConnection
{
      public:
    virtual ~HardwareConnection()
    {
    }
    virtual int pause() = 0;
    virtual int resume() = 0;
};
