/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/

#ifndef SCHEDULER_APPC_H_ 
#define SCHEDULER_APPC_H_


#include <stdint.h>

#ifdef   __cplusplus
extern "C"
{
#endif

#include "cstream_node.h"


/* Node identifiers */
#define STREAM_APPC_NB_IDENTIFIED_NODES 2
#define STREAM_APPC_LCD_ID 0
#define STREAM_APPC_VIDEO_ID 1

#define STREAM_APPC_SCHED_LEN 0


extern CStreamNode* get_scheduler_appc_node(int32_t nodeID);

extern int init_scheduler_appc(void *evtQueue_,AppcParams *params);
extern void free_scheduler_appc();
extern uint32_t scheduler_appc(int *error);
extern void reset_fifos_scheduler_appc(int all);

#ifdef   __cplusplus
}
#endif

#endif

