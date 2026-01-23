#ifndef APPC_PARAMS_H
#define APPC_PARAMS_H

#include "node_settings_datatype.h"


#ifdef   __cplusplus
extern "C"
{
#endif

struct AppcParams
{
    struct hardwareParams hw_;
};

extern struct AppcParams appcParams;

#ifdef   __cplusplus
}
#endif

#endif