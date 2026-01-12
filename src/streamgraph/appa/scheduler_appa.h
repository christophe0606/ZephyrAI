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
#define STREAMNB_IDENTIFIED_NODES 11
#define STREAMAUDIOSOURCE_ID 0
#define STREAMAUDIOWIN_ID 1
#define STREAMDEINTERLEAVE_ID 2
#define STREAMMFCC_ID 3
#define STREAMMFCCWIN_ID 4
#define STREAMNULLRIGHT_ID 5
#define STREAMSEND_ID 6
#define STREAMTO_F32_ID 7
#define STREAMCLASSIFY_ID 8
#define STREAMDISPLAY_ID 9
#define STREAMKWS_ID 10


extern CStreamNode* get_scheduler_appa_node(int32_t nodeID);

extern int init_scheduler_appa(void *evtQueue_,AppaParams *params);
extern void free_scheduler_appa();
extern uint32_t scheduler_appa(int *error);
extern void reset_fifos_scheduler_appa(int all);

#ifdef   __cplusplus
}
#endif

#endif

