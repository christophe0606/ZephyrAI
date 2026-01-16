/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/

#ifndef SCHEDULER_APPA_H_ 
#define SCHEDULER_APPA_H_


#include <stdint.h>

#ifdef   __cplusplus
extern "C"
{
#endif

#include "cstream_node.h"


/* Node identifiers */
#define STREAM_APPA_NB_IDENTIFIED_NODES 2
#define STREAM_APPA_AUDIOSOURCE_ID 0
#define STREAM_APPA_DISPLAY_ID 1


extern CStreamNode* get_scheduler_appa_node(int32_t nodeID);

extern int init_scheduler_appa(void *evtQueue_,AppaParams *params);
extern void free_scheduler_appa();
extern uint32_t scheduler_appa(int *error);
extern void reset_fifos_scheduler_appa(int all);

#ifdef   __cplusplus
}
#endif

#endif

