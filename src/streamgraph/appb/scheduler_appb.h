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
#define STREAM_APPB_NB_IDENTIFIED_NODES 15
#define STREAM_APPB_AUDIO_ID 0
#define STREAM_APPB_AUDIOWINLEFT_ID 1
#define STREAM_APPB_AUDIOWINRIGHT_ID 2
#define STREAM_APPB_DEINTERLEAVE_ID 3
#define STREAM_APPB_FFTLEFT_ID 4
#define STREAM_APPB_FFTRIGHT_ID 5
#define STREAM_APPB_GAIN_ID 6
#define STREAM_APPB_SPECTROGRAMLEFT_ID 7
#define STREAM_APPB_SPECTROGRAMRIGHT_ID 8
#define STREAM_APPB_TOCOMPLEXLEFT_ID 9
#define STREAM_APPB_TOCOMPLEXRIGHT_ID 10
#define STREAM_APPB_TO_F32_ID 11
#define STREAM_APPB_WINLEFT_ID 12
#define STREAM_APPB_WINRIGHT_ID 13
#define STREAM_APPB_DISPLAY_ID 14


extern CStreamNode* get_scheduler_appb_node(int32_t nodeID);

extern int init_scheduler_appb(void *evtQueue_,AppbParams *params);
extern void free_scheduler_appb();
extern uint32_t scheduler_appb(int *error);
extern void reset_fifos_scheduler_appb(int all);

#ifdef   __cplusplus
}
#endif

#endif

