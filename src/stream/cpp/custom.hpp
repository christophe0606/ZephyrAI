/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Library
 * Title:        custom.h
 * Description:  Example configuration for CMSIS-Stream with event handling in bare metal
 *               and multi-threaded environments.
 *
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * --------------------------------------------------------------------
 *
 * Copyright (C) 2021-2025 ARM Limited or its affiliates. All rights reserved.
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
#ifndef CUSTOM_H_
#define CUSTOM_H_

#include "arm_mve.h"

extern "C"
{
#include <zephyr/kernel.h>
#include "config.h"

extern struct k_event cg_streamEvent;

}

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

class ZephyrMutex
{
  public:
    ZephyrMutex()
    {
        k_mutex_init(&my_mutex);
    }

    ~ZephyrMutex()
    {
        // No explicit deinitialization needed for k_mutex
    }

    struct k_mutex* id() 
    {
        return &my_mutex;
    }

  protected:
    struct k_mutex my_mutex;
};

class ZephyrLock
{
  public:
    ZephyrLock(ZephyrMutex &mutex)
        : mutex(mutex)
    {
    }

    int acquire()
    {
        error = k_mutex_lock(mutex.id(), K_FOREVER);
        return error;
    }

    int tryAcquire()
    {
        error = k_mutex_lock(mutex.id(), K_NO_WAIT);
        return error;
    }

    ~ZephyrLock()
    {
        if (error == 0)
        {
             k_mutex_unlock(mutex.id());
        }
    }

    int getError() const
    {
        return error;
    }

  protected:
    ZephyrMutex &mutex;
    int error;
};

#define CG_MUTEX ZephyrMutex
#define CG_MUTEX_ERROR_TYPE int

#define CG_MUTEX_HAS_ERROR(ERROR) (ERROR != 0)

#define CG_ENTER_CRITICAL_SECTION(MUTEX, ERROR) \
    {                                           \
        ZephyrLock lock((MUTEX));                \
        ERROR = lock.acquire();

#define CG_EXIT_CRITICAL_SECTION(MUTEX, ERROR) \
    }

#define CG_ENTER_READ_CRITICAL_SECTION(MUTEX, ERROR) \
    {                                                \
        ZephyrLock lock((MUTEX));                     \
        ERROR = lock.acquire();

#define CG_EXIT_READ_CRITICAL_SECTION(MUTEX, ERROR) \
    }

#define CG_MK_LIST_EVENT_ALLOCATOR(T) (ZephyrEventPoolAllocator<T>{})
#define CG_MK_PROTECTED_BUF_ALLOCATOR(T) (ZephyrBufPoolAllocator<T>{})
#define CG_MK_PROTECTED_MUTEX_ALLOCATOR(T) (ZephyrMutexPoolAllocator<T>{})

#include "zephyr_allocator.hpp"



#include "rtos_events.hpp"

// Because memory optimization is enabled
#define CG_BEFORE_BUFFER __ALIGNED(16)

#define CG_BEFORE_SCHEDULE \
  uint32_t errorFlags = 0;

  // When real audio events are generated the NO_WAIT
  // must be replaced with forever
#define CG_BEFORE_NODE_EXECUTION(ID)                                                                                       \
{                                                                                                                          \
    errorFlags = k_event_wait(&cg_streamEvent,AUDIO_SINK_UNDERFLOW_EVENT | AUDIO_SOURCE_OVERFLOW_EVENT, false, K_NO_WAIT); \
    if (errorFlags & AUDIO_SOURCE_OVERFLOW_EVENT)                                                                          \
    {                                                                                                                      \
        cgStaticError = CG_BUFFER_OVERFLOW;                                                                                \
        goto errorHandling;                                                                                                \
    }                                                                                                                      \
}                                                                                                                          


#define CG_TIME_STAMP_TYPE uint32_t

#define CG_GET_TIME_STAMP()  k_cycle_get_32()  


// Queue implementation for events
#include "cg_queue.hpp"


#endif