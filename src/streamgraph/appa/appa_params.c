#include <stdlib.h>
#include <stdint.h>
#include "appa_params.h"


struct AppaParams appaParams = {
    // Initialize parameters for each node as needed
    .classify = {
        .historyLength = 10, // Example value
    },
    .kws = {
        .modelAddr = NULL, // To be set to the model address
        .modelSize = 0       // To be set to the model size
    }
};  