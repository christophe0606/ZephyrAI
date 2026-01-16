/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/

#ifndef SCHEDULER_APPB_H_ 
#define SCHEDULER_APPB_H_


#include <stdint.h>

#ifdef   __cplusplus
extern "C"
{
#endif

#include "cstream_node.h"


/* Node identifiers */
#define STREAM_APPB_NB_IDENTIFIED_NODES 2
#define STREAM_APPB_AUDIO_ID 0
#define STREAM_APPB_DISPLAY_ID 1


extern CStreamNode* get_scheduler_appb_node(int32_t nodeID);

extern int init_scheduler_appb(void *evtQueue_,AppbParams *params);
extern void free_scheduler_appb();
extern uint32_t scheduler_appb(int *error);
extern void reset_fifos_scheduler_appb(int all);

#ifdef   __cplusplus
}
#endif

#endif

