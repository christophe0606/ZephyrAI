/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/

#ifndef _SCHEDULER_H_ 
#define _SCHEDULER_H_

#ifdef   __cplusplus
extern "C"
{
#endif


/* Node identifiers */
#define STREAMNB_IDENTIFIED_NODES 14
#define STREAMAUDIO_ID 0
#define STREAMAUDIOWINLEFT_ID 1
#define STREAMAUDIOWINRIGHT_ID 2
#define STREAMDEINTERLEAVE_ID 3
#define STREAMFFTLEFT_ID 4
#define STREAMFFTRIGHT_ID 5
#define STREAMSPECTROGRAMLEFT_ID 6
#define STREAMSPECTROGRAMRIGHT_ID 7
#define STREAMTOCOMPLEXLEFT_ID 8
#define STREAMTOCOMPLEXRIGHT_ID 9
#define STREAMTO_F32_ID 10
#define STREAMWINLEFT_ID 11
#define STREAMWINRIGHT_ID 12
#define STREAMDISPLAY_ID 13


extern CStreamNode* get_scheduler_node(int32_t nodeID);


extern int init_scheduler();
extern void free_scheduler();

extern uint32_t scheduler(int *error);

#ifdef   __cplusplus
}
#endif

#endif

