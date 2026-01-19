#include <stdlib.h>
#include <stdint.h>
#include "appb_params.h"

struct AppbParams appbParams = {
    .hw_ = {
        .i2s_mic = NULL, // To be set to the I2S device
        .mem_slab = NULL // To be set to the memory slab
    }
}; 