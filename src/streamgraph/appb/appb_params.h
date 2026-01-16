#ifndef APPB_PARAMS_H
#define APPB_PARAMS_H

#include "node_settings_datatype.h"


#ifdef   __cplusplus
extern "C"
{
#endif

struct AppbParams
{
    struct zephyrAudioSourceParams audio;
};

extern struct AppbParams appbParams;

#ifdef   __cplusplus
}
#endif

#endif