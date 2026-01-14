/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/


#include <cstdint>
#include "app_config.hpp"
#include "stream_platform_config.hpp"
#include "cg_enums.h"
#include "StreamNode.hpp"
#include "cstream_node.h"
#include "IdentifiedNode.hpp"
#include "EventQueue.hpp"
#include "GenericNodes.hpp"
#include "AppNodes_appa.hpp"
#include "scheduler_appa.h"
#include "appa_extern_templates.hpp"

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
static uint8_t schedule[8]=
{ 
0,2,5,7,1,3,4,6,
};

/*

Internal ID identification for the nodes

*/
#define AUDIOSOURCE_INTERNAL_ID 0
#define AUDIOWIN_INTERNAL_ID 1
#define DEINTERLEAVE_INTERNAL_ID 2
#define MFCC_INTERNAL_ID 3
#define MFCCWIN_INTERNAL_ID 4
#define NULLRIGHT_INTERNAL_ID 5
#define SEND_INTERNAL_ID 6
#define TO_F32_INTERNAL_ID 7
#define CLASSIFY_INTERNAL_ID 8
#define DISPLAY_INTERNAL_ID 9
#define KWS_INTERNAL_ID 10



/***********

Node identification

************/
static CStreamNode identifiedNodes[STREAM_APPA_NB_IDENTIFIED_NODES]={0};

CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/
#define FIFOSIZE0 320
#define FIFOSIZE1 320
#define FIFOSIZE2 320
#define FIFOSIZE3 640
#define FIFOSIZE4 10
#define FIFOSIZE5 490
#define FIFOSIZE6 320

#define BUFFERSIZE0 2560
CG_BEFORE_BUFFER
uint8_t stream_appa_buf0[BUFFERSIZE0]={0};

#define BUFFERSIZE1 1280
CG_BEFORE_BUFFER
uint8_t stream_appa_buf1[BUFFERSIZE1]={0};

#define BUFFERSIZE2 640
CG_BEFORE_BUFFER
uint8_t stream_appa_buf2[BUFFERSIZE2]={0};


typedef struct {
FIFO<sq15,FIFOSIZE0,1,0> *fifo0;
FIFO<q15_t,FIFOSIZE1,1,0> *fifo1;
FIFO<float,FIFOSIZE2,1,0> *fifo2;
FIFO<float,FIFOSIZE3,1,0> *fifo3;
FIFO<float,FIFOSIZE4,1,0> *fifo4;
FIFO<float,FIFOSIZE5,1,0> *fifo5;
FIFO<q15_t,FIFOSIZE6,1,0> *fifo6;
} fifos_t;

typedef struct {
    ZephyrAudioSource<sq15,320> *audioSource;
    SlidingBuffer<float,640,320> *audioWin;
    DeinterleaveStereo<sq15,320,q15_t,320,q15_t,320> *deinterleave;
    MFCC<float,640,float,10> *mfcc;
    SlidingBuffer<float,490,480> *mfccWin;
    NullSink<q15_t,320> *nullRight;
    SendToNetwork<float,490> *send;
    Convert<q15_t,320,float,320> *to_f32;
    KWSClassify *classify;
    KWSDisplay *display;
    KWS *kws;
} nodes_t;


static fifos_t fifos={0};

static nodes_t nodes={0};

CStreamNode* get_scheduler_appa_node(int32_t nodeID)
{
    if (nodeID >= STREAM_APPA_NB_IDENTIFIED_NODES)
    {
        return(nullptr);
    }
    if (nodeID < 0)
    {
        return(nullptr);
    }
    return(&identifiedNodes[nodeID]);
}

int init_scheduler_appa(void *evtQueue_,AppaParams *params)
{
    EventQueue *evtQueue = reinterpret_cast<EventQueue *>(evtQueue_);

    CG_BEFORE_FIFO_INIT;
    fifos.fifo0 = new (std::nothrow) FIFO<sq15,FIFOSIZE0,1,0>(stream_appa_buf1);
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo1 = new (std::nothrow) FIFO<q15_t,FIFOSIZE1,1,0>(stream_appa_buf0);
    if (fifos.fifo1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo2 = new (std::nothrow) FIFO<float,FIFOSIZE2,1,0>(stream_appa_buf1);
    if (fifos.fifo2==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo3 = new (std::nothrow) FIFO<float,FIFOSIZE3,1,0>(stream_appa_buf0);
    if (fifos.fifo3==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo4 = new (std::nothrow) FIFO<float,FIFOSIZE4,1,0>(stream_appa_buf1);
    if (fifos.fifo4==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo5 = new (std::nothrow) FIFO<float,FIFOSIZE5,1,0>(stream_appa_buf0);
    if (fifos.fifo5==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo6 = new (std::nothrow) FIFO<q15_t,FIFOSIZE6,1,0>(stream_appa_buf2);
    if (fifos.fifo6==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    cg_status initError;

    nodes.audioSource = new (std::nothrow) ZephyrAudioSource<sq15,320>(*(fifos.fifo0));
    if (nodes.audioSource==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_AUDIOSOURCE_ID]=createStreamNode(*nodes.audioSource);
    nodes.audioSource->setID(STREAM_APPA_AUDIOSOURCE_ID);

    nodes.audioWin = new (std::nothrow) SlidingBuffer<float,640,320>(*(fifos.fifo2),*(fifos.fifo3));
    if (nodes.audioWin==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_AUDIOWIN_ID]=createStreamNode(*nodes.audioWin);
    nodes.audioWin->setID(STREAM_APPA_AUDIOWIN_ID);

    nodes.deinterleave = new (std::nothrow) DeinterleaveStereo<sq15,320,q15_t,320,q15_t,320>(*(fifos.fifo0),*(fifos.fifo1),*(fifos.fifo6));
    if (nodes.deinterleave==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_DEINTERLEAVE_ID]=createStreamNode(*nodes.deinterleave);
    nodes.deinterleave->setID(STREAM_APPA_DEINTERLEAVE_ID);

    nodes.mfcc = new (std::nothrow) MFCC<float,640,float,10>(*(fifos.fifo3),*(fifos.fifo4));
    if (nodes.mfcc==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_MFCC_ID]=createStreamNode(*nodes.mfcc);
    nodes.mfcc->setID(STREAM_APPA_MFCC_ID);

    nodes.mfccWin = new (std::nothrow) SlidingBuffer<float,490,480>(*(fifos.fifo4),*(fifos.fifo5));
    if (nodes.mfccWin==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_MFCCWIN_ID]=createStreamNode(*nodes.mfccWin);
    nodes.mfccWin->setID(STREAM_APPA_MFCCWIN_ID);

    nodes.nullRight = new (std::nothrow) NullSink<q15_t,320>(*(fifos.fifo6),evtQueue);
    if (nodes.nullRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_NULLRIGHT_ID]=createStreamNode(*nodes.nullRight);
    nodes.nullRight->setID(STREAM_APPA_NULLRIGHT_ID);

    nodes.send = new (std::nothrow) SendToNetwork<float,490>(*(fifos.fifo5),evtQueue);
    if (nodes.send==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_SEND_ID]=createStreamNode(*nodes.send);
    nodes.send->setID(STREAM_APPA_SEND_ID);

    nodes.to_f32 = new (std::nothrow) Convert<q15_t,320,float,320>(*(fifos.fifo1),*(fifos.fifo2));
    if (nodes.to_f32==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_TO_F32_ID]=createStreamNode(*nodes.to_f32);
    nodes.to_f32->setID(STREAM_APPA_TO_F32_ID);

    nodes.classify = new (std::nothrow) KWSClassify(evtQueue,params->classify.historyLength);
    if (nodes.classify==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_CLASSIFY_ID]=createStreamNode(*nodes.classify);
    nodes.classify->setID(STREAM_APPA_CLASSIFY_ID);

    nodes.display = new (std::nothrow) KWSDisplay(evtQueue);
    if (nodes.display==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_DISPLAY_ID]=createStreamNode(*nodes.display);
    nodes.display->setID(STREAM_APPA_DISPLAY_ID);

    nodes.kws = new (std::nothrow) KWS(evtQueue,params->kws.modelAddr,params->kws.modelSize);
    if (nodes.kws==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPA_KWS_ID]=createStreamNode(*nodes.kws);
    nodes.kws->setID(STREAM_APPA_KWS_ID);


/* Subscribe nodes for the event system*/
    nodes.send->subscribe(0,*nodes.kws,0);
    nodes.kws->subscribe(0,*nodes.send,0);
    nodes.kws->subscribe(1,*nodes.classify,0);
    nodes.classify->subscribe(0,*nodes.display,0);

    initError = CG_SUCCESS;
    initError = nodes.audioSource->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.audioWin->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.deinterleave->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.mfcc->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.mfccWin->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.nullRight->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.send->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.to_f32->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.classify->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.display->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.kws->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
   


    return(CG_SUCCESS);

}

void free_scheduler_appa()
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
    if (fifos.fifo4!=NULL)
    {
       delete fifos.fifo4;
    }
    if (fifos.fifo5!=NULL)
    {
       delete fifos.fifo5;
    }
    if (fifos.fifo6!=NULL)
    {
       delete fifos.fifo6;
    }

    if (nodes.audioSource!=NULL)
    {
        delete nodes.audioSource;
    }
    if (nodes.audioWin!=NULL)
    {
        delete nodes.audioWin;
    }
    if (nodes.deinterleave!=NULL)
    {
        delete nodes.deinterleave;
    }
    if (nodes.mfcc!=NULL)
    {
        delete nodes.mfcc;
    }
    if (nodes.mfccWin!=NULL)
    {
        delete nodes.mfccWin;
    }
    if (nodes.nullRight!=NULL)
    {
        delete nodes.nullRight;
    }
    if (nodes.send!=NULL)
    {
        delete nodes.send;
    }
    if (nodes.to_f32!=NULL)
    {
        delete nodes.to_f32;
    }
    if (nodes.classify!=NULL)
    {
        delete nodes.classify;
    }
    if (nodes.display!=NULL)
    {
        delete nodes.display;
    }
    if (nodes.kws!=NULL)
    {
        delete nodes.kws;
    }
}

void reset_fifos_scheduler_appa(int all)
{
    if (fifos.fifo0!=NULL)
    {
       fifos.fifo0->reset();
    }
    if (fifos.fifo1!=NULL)
    {
       fifos.fifo1->reset();
    }
    if (fifos.fifo2!=NULL)
    {
       fifos.fifo2->reset();
    }
    if (fifos.fifo3!=NULL)
    {
       fifos.fifo3->reset();
    }
    if (fifos.fifo4!=NULL)
    {
       fifos.fifo4->reset();
    }
    if (fifos.fifo5!=NULL)
    {
       fifos.fifo5->reset();
    }
    if (fifos.fifo6!=NULL)
    {
       fifos.fifo6->reset();
    }
   // Buffers are set to zero too
   if (all)
   {
       std::fill_n(stream_appa_buf0, BUFFERSIZE0, (uint8_t)0);
       std::fill_n(stream_appa_buf1, BUFFERSIZE1, (uint8_t)0);
       std::fill_n(stream_appa_buf2, BUFFERSIZE2, (uint8_t)0);
   }
}


CG_BEFORE_SCHEDULER_FUNCTION
uint32_t scheduler_appa(int *error)
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
        for(; id < 8; id++)
        {
            CG_BEFORE_NODE_EXECUTION(schedule[id]);
            switch(schedule[id])
            {
                case 0:
                {
                    
                   cgStaticError = nodes.audioSource->run();
                }
                break;

                case 1:
                {
                    
                   cgStaticError = nodes.audioWin->run();
                }
                break;

                case 2:
                {
                    
                   cgStaticError = nodes.deinterleave->run();
                }
                break;

                case 3:
                {
                    
                   cgStaticError = nodes.mfcc->run();
                }
                break;

                case 4:
                {
                    
                   cgStaticError = nodes.mfccWin->run();
                }
                break;

                case 5:
                {
                    
                   cgStaticError = nodes.nullRight->run();
                }
                break;

                case 6:
                {
                    
                   cgStaticError = nodes.send->run();
                }
                break;

                case 7:
                {
                    
                   cgStaticError = nodes.to_f32->run();
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
