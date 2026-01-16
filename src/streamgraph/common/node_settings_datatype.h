#ifndef NODE_SETTINGS_DATATYPE_H
#define NODE_SETTINGS_DATATYPE_H

#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>

struct classifyParams
{
    int historyLength;
};

struct tfliteNodeParams
{
   uint8_t *modelAddr;
   size_t modelSize;
};

struct zephyrAudioSourceParams
{
   const struct device *i2s_mic;
   struct k_mem_slab *mem_slab;
};

#endif