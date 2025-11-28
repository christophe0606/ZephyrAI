#pragma once 

#include <cstdint>
#include "ethos_def.hpp"

typedef struct
{
    volatile uint32_t currentIndex;
    volatile uint32_t transmitIndex;
    uint32_t bufferSize;
    uint8_t *audioBuffer;
} audioOutputGlobalState_t;

extern audioOutputGlobalState_t g_audioOutputState;

// If graph contains a node named TFLITE
extern uint8_t tensorArena[ACTIVATION_BUF_SZ] ACTIVATION_BUF_ATTRIBUTE;
