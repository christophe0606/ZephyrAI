/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/


#include <cstdint>
#include "custom.hpp"
#include "cg_enums.h"
#include "StreamNode.hpp"
#include "cstream_node.h"
#include "IdentifiedNode.hpp"
#include "EventQueue.hpp"
#include "GenericNodes.hpp"
#include "AppNodes.hpp"
#include "scheduler.h"

#if !defined(CHECKERROR)
#define CHECKERROR       if (cgStaticError < 0) \
       {\
         goto errorHandling;\
       }

#endif


#if !defined(CG_BEFORE_ITERATION)
#define CG_BEFORE_ITERATION
#endif 

#if !defined(CG_AFTER_ITERATION)
#define CG_AFTER_ITERATION
#endif 

#if !defined(CG_BEFORE_SCHEDULE)
#define CG_BEFORE_SCHEDULE
#endif

#if !defined(CG_AFTER_SCHEDULE)
#define CG_AFTER_SCHEDULE
#endif

#if !defined(CG_BEFORE_BUFFER)
#define CG_BEFORE_BUFFER
#endif

#if !defined(CG_BEFORE_FIFO_BUFFERS)
#define CG_BEFORE_FIFO_BUFFERS
#endif

#if !defined(CG_BEFORE_FIFO_INIT)
#define CG_BEFORE_FIFO_INIT
#endif

#if !defined(CG_BEFORE_NODE_INIT)
#define CG_BEFORE_NODE_INIT
#endif

#if !defined(CG_AFTER_INCLUDES)
#define CG_AFTER_INCLUDES
#endif

#if !defined(CG_BEFORE_SCHEDULER_FUNCTION)
#define CG_BEFORE_SCHEDULER_FUNCTION
#endif

#if !defined(CG_BEFORE_NODE_EXECUTION)
#define CG_BEFORE_NODE_EXECUTION(ID)
#endif

#if !defined(CG_AFTER_NODE_EXECUTION)
#define CG_AFTER_NODE_EXECUTION(ID)
#endif





CG_AFTER_INCLUDES


using namespace arm_cmsis_stream;

/*

Description of the scheduling. 

*/
static uint8_t schedule[4]=
{ 
0,1,3,2,
};

/*

Internal ID identification for the nodes

*/
#define AUDIO_INTERNAL_ID 0
#define DEINTERLEAVE_INTERNAL_ID 1
#define NULLSINK_INTERNAL_ID 2
#define STEREOTOMONO_INTERNAL_ID 3



/***********

Node identification

************/
static CStreamNode identifiedNodes[NB_IDENTIFIED_NODES]={0};

CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/
#define FIFOSIZE0 320
#define FIFOSIZE1 320
#define FIFOSIZE2 320
#define FIFOSIZE3 320

#define BUFFERSIZE0 640
CG_BEFORE_BUFFER
uint8_t buf0[BUFFERSIZE0]={0};

#define BUFFERSIZE1 640
CG_BEFORE_BUFFER
uint8_t buf1[BUFFERSIZE1]={0};

#define BUFFERSIZE2 1280
CG_BEFORE_BUFFER
uint8_t buf2[BUFFERSIZE2]={0};


typedef struct {
FIFO<sq15,FIFOSIZE0,1,0> *fifo0;
FIFO<q15_t,FIFOSIZE1,1,0> *fifo1;
FIFO<q15_t,FIFOSIZE2,1,0> *fifo2;
FIFO<q15_t,FIFOSIZE3,1,0> *fifo3;
} fifos_t;

typedef struct {
    ZephyrAudioSource<sq15,320> *audio;
    DeinterleaveStereo<sq15,320,q15_t,320,q15_t,320> *deinterleave;
    NullSink<q15_t,320> *nullSink;
    StereoToMono<q15_t,320,q15_t,320,q15_t,320> *stereoToMono;
} nodes_t;


static fifos_t fifos={0};

static nodes_t nodes={0};

CStreamNode* get_scheduler_node(int32_t nodeID)
{
    if (nodeID >= NB_IDENTIFIED_NODES)
    {
        return(nullptr);
    }
    if (nodeID < 0)
    {
        return(nullptr);
    }
    return(&identifiedNodes[nodeID]);
}

int init_scheduler()
{

    CG_BEFORE_FIFO_INIT;
    fifos.fifo0 = new (std::nothrow) FIFO<sq15,FIFOSIZE0,1,0>(buf2);
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo1 = new (std::nothrow) FIFO<q15_t,FIFOSIZE1,1,0>(buf0);
    if (fifos.fifo1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo2 = new (std::nothrow) FIFO<q15_t,FIFOSIZE2,1,0>(buf1);
    if (fifos.fifo2==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo3 = new (std::nothrow) FIFO<q15_t,FIFOSIZE3,1,0>(buf2);
    if (fifos.fifo3==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    cg_status initError;

    nodes.audio = new (std::nothrow) ZephyrAudioSource<sq15,320>(*(fifos.fifo0));
    if (nodes.audio==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[AUDIO_ID]=createStreamNode(*nodes.audio);
    nodes.audio->setID(AUDIO_ID);

    nodes.deinterleave = new (std::nothrow) DeinterleaveStereo<sq15,320,q15_t,320,q15_t,320>(*(fifos.fifo0),*(fifos.fifo1),*(fifos.fifo2));
    if (nodes.deinterleave==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[DEINTERLEAVE_ID]=createStreamNode(*nodes.deinterleave);
    nodes.deinterleave->setID(DEINTERLEAVE_ID);

    nodes.nullSink = new (std::nothrow) NullSink<q15_t,320>(*(fifos.fifo3));
    if (nodes.nullSink==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[NULLSINK_ID]=createStreamNode(*nodes.nullSink);
    nodes.nullSink->setID(NULLSINK_ID);

    nodes.stereoToMono = new (std::nothrow) StereoToMono<q15_t,320,q15_t,320,q15_t,320>(*(fifos.fifo1),*(fifos.fifo2),*(fifos.fifo3));
    if (nodes.stereoToMono==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STEREOTOMONO_ID]=createStreamNode(*nodes.stereoToMono);
    nodes.stereoToMono->setID(STEREOTOMONO_ID);


/* Subscribe nodes for the event system*/

    initError = CG_SUCCESS;
    initError = nodes.audio->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.deinterleave->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.nullSink->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.stereoToMono->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
   


    return(CG_SUCCESS);

}

void free_scheduler()
{
    if (fifos.fifo0!=NULL)
    {
       delete fifos.fifo0;
    }
    if (fifos.fifo1!=NULL)
    {
       delete fifos.fifo1;
    }
    if (fifos.fifo2!=NULL)
    {
       delete fifos.fifo2;
    }
    if (fifos.fifo3!=NULL)
    {
       delete fifos.fifo3;
    }

    if (nodes.audio!=NULL)
    {
        delete nodes.audio;
    }
    if (nodes.deinterleave!=NULL)
    {
        delete nodes.deinterleave;
    }
    if (nodes.nullSink!=NULL)
    {
        delete nodes.nullSink;
    }
    if (nodes.stereoToMono!=NULL)
    {
        delete nodes.stereoToMono;
    }
}


CG_BEFORE_SCHEDULER_FUNCTION
uint32_t scheduler(int *error)
{
    int cgStaticError=0;
    uint32_t nbSchedule=0;





    /* Run several schedule iterations */
    CG_BEFORE_SCHEDULE;
    while(cgStaticError==0)
    {
        /* Run a schedule iteration */
        CG_BEFORE_ITERATION;
        unsigned long id=0;
        for(; id < 4; id++)
        {
            CG_BEFORE_NODE_EXECUTION(schedule[id]);
            switch(schedule[id])
            {
                case 0:
                {
                    
                   cgStaticError = nodes.audio->run();
                }
                break;

                case 1:
                {
                    
                   cgStaticError = nodes.deinterleave->run();
                }
                break;

                case 2:
                {
                    
                   cgStaticError = nodes.nullSink->run();
                }
                break;

                case 3:
                {
                    
                   cgStaticError = nodes.stereoToMono->run();
                }
                break;

                default:
                break;
            }
            CG_AFTER_NODE_EXECUTION(schedule[id]);
                        CHECKERROR;
        }
       CG_AFTER_ITERATION;
       nbSchedule++;
    }
errorHandling:
    CG_AFTER_SCHEDULE;
    *error=cgStaticError;
    return(nbSchedule);
    
}
