#ifndef NODE_SETTINGS_DATATYPE_H
#define NODE_SETTINGS_DATATYPE_H

#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>

#ifdef   __cplusplus
extern "C"
{
#endif

struct classifyParams
{
    int historyLength;
};

struct tfliteNodeParams
{
   uint8_t *modelAddr;
   size_t modelSize;
};

/**
 * @brief Structure to hold hardware connection parameters
 * for nodes that interact with hardware components.
 * The convention is that each parameter structure for a graph
 * starts with a hw_ field of type hardwareParams.
 */
struct hardwareParams
{
   const struct device *i2s_mic;
   struct k_mem_slab *mem_slab;
};

#ifdef   __cplusplus
}
#endif

#endif