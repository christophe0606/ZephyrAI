#include <stdlib.h>
#include <stdint.h>
#include "grapha_params.h"


struct GraphaParams graphaParams = {
    // Initialize parameters for each node as needed
    .classify = {
        .historyLength = 10, // Example value
    },
    .kws = {
        .modelAddr = NULL, // To be set to the model address
        .modelSize = 0       // To be set to the model size
    }
};  