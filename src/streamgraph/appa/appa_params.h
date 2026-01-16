#ifndef APPA_PARAMS_H
#define APPA_PARAMS_H

#include "node_settings_datatype.h"

#ifdef   __cplusplus
extern "C"
{
#endif


struct AppaParams
{
    // Name of struct is the name of the node as defined
    // in Python graph.
    struct classifyParams classify;
    struct tfliteNodeParams kws;
    struct zephyrAudioSourceParams audioSource;
};

extern struct AppaParams appaParams;

#ifdef   __cplusplus
}
#endif

#endif