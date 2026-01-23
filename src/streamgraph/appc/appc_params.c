#include <stdlib.h>
#include <stdint.h>
#include "appc_params.h"

struct AppcParams appcParams = {
    .hw_ = {
        .i2s_mic = NULL, // To be set to the I2S device
        .mem_slab = NULL // To be set to the memory slab
    }
}; 