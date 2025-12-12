#pragma once 

//#include "arm_mve.h"
#include "arm_math_types.h"

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

// Needed for pure event graphs to avoid having
// the infinite loop generated in single thread mode
// to process events from the stream thread.
#define CG_EVENTS_MULTI_THREAD

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
#define CG_BEFORE_BUFFER __aligned(16)

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


#define CG_TIME_STAMP_TYPE uint32_t

#define CG_GET_TIME_STAMP()  k_cycle_get_32()  
