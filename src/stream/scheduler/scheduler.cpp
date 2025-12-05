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
static uint8_t schedule[9]=
{ 
0,7,2,4,1,8,6,3,5,
};

/*

Internal ID identification for the nodes

*/
#define AUDIO_INTERNAL_ID 0
#define AUDIOWINLEFT_INTERNAL_ID 1
#define DEINTERLEAVE_INTERNAL_ID 2
#define FFTLEFT_INTERNAL_ID 3
#define NULLSINKRIGHT_INTERNAL_ID 4
#define SPECTROGRAMLEFT_INTERNAL_ID 5
#define TOCOMPLEXLEFT_INTERNAL_ID 6
#define TO_F32_INTERNAL_ID 7
#define WINLEFT_INTERNAL_ID 8
#define DISPLAY_INTERNAL_ID 9



/***********

Node identification

************/
static CStreamNode identifiedNodes[STREAMNB_IDENTIFIED_NODES]={0};

CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/
#define FIFOSIZE0 256
#define FIFOSIZE1 256
#define FIFOSIZE2 256
#define FIFOSIZE3 512
#define FIFOSIZE4 512
#define FIFOSIZE5 512
#define FIFOSIZE6 512
#define FIFOSIZE7 256

#define BUFFERSIZE0 4096
CG_BEFORE_BUFFER
uint8_t streambuf0[BUFFERSIZE0]={0};

#define BUFFERSIZE1 4096
CG_BEFORE_BUFFER
uint8_t streambuf1[BUFFERSIZE1]={0};

#define BUFFERSIZE2 1024
CG_BEFORE_BUFFER
uint8_t streambuf2[BUFFERSIZE2]={0};


typedef struct {
FIFO<sq15,FIFOSIZE0,1,0> *fifo0;
FIFO<sf32,FIFOSIZE1,1,0> *fifo1;
FIFO<float,FIFOSIZE2,1,0> *fifo2;
FIFO<float,FIFOSIZE3,1,0> *fifo3;
FIFO<float,FIFOSIZE4,1,0> *fifo4;
FIFO<cf32,FIFOSIZE5,1,0> *fifo5;
FIFO<cf32,FIFOSIZE6,1,0> *fifo6;
FIFO<float,FIFOSIZE7,1,0> *fifo7;
} fifos_t;

typedef struct {
    ZephyrAudioSource<sq15,256> *audio;
    SlidingBuffer<float,512,256> *audioWinLeft;
    DeinterleaveStereo<sf32,256,float,256,float,256> *deinterleave;
    CFFT<cf32,512,cf32,512> *fftLeft;
    NullSink<float,256> *nullSinkRight;
    Spectrogram<cf32,512> *spectrogramLeft;
    RealToComplex<float,512,cf32,512> *toComplexLeft;
    Convert<sq15,256,sf32,256> *to_f32;
    Hanning<float,512,float,512> *winLeft;
    AppDisplay *display;
} nodes_t;


static fifos_t fifos={0};

static nodes_t nodes={0};

CStreamNode* get_scheduler_node(int32_t nodeID)
{
    if (nodeID >= STREAMNB_IDENTIFIED_NODES)
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
    fifos.fifo0 = new (std::nothrow) FIFO<sq15,FIFOSIZE0,1,0>(streambuf1);
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo1 = new (std::nothrow) FIFO<sf32,FIFOSIZE1,1,0>(streambuf0);
    if (fifos.fifo1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo2 = new (std::nothrow) FIFO<float,FIFOSIZE2,1,0>(streambuf1);
    if (fifos.fifo2==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo3 = new (std::nothrow) FIFO<float,FIFOSIZE3,1,0>(streambuf0);
    if (fifos.fifo3==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo4 = new (std::nothrow) FIFO<float,FIFOSIZE4,1,0>(streambuf1);
    if (fifos.fifo4==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo5 = new (std::nothrow) FIFO<cf32,FIFOSIZE5,1,0>(streambuf0);
    if (fifos.fifo5==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo6 = new (std::nothrow) FIFO<cf32,FIFOSIZE6,1,0>(streambuf1);
    if (fifos.fifo6==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo7 = new (std::nothrow) FIFO<float,FIFOSIZE7,1,0>(streambuf2);
    if (fifos.fifo7==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    cg_status initError;

    nodes.audio = new (std::nothrow) ZephyrAudioSource<sq15,256>(*(fifos.fifo0));
    if (nodes.audio==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMAUDIO_ID]=createStreamNode(*nodes.audio);
    nodes.audio->setID(STREAMAUDIO_ID);

    nodes.audioWinLeft = new (std::nothrow) SlidingBuffer<float,512,256>(*(fifos.fifo2),*(fifos.fifo3));
    if (nodes.audioWinLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMAUDIOWINLEFT_ID]=createStreamNode(*nodes.audioWinLeft);
    nodes.audioWinLeft->setID(STREAMAUDIOWINLEFT_ID);

    nodes.deinterleave = new (std::nothrow) DeinterleaveStereo<sf32,256,float,256,float,256>(*(fifos.fifo1),*(fifos.fifo2),*(fifos.fifo7));
    if (nodes.deinterleave==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMDEINTERLEAVE_ID]=createStreamNode(*nodes.deinterleave);
    nodes.deinterleave->setID(STREAMDEINTERLEAVE_ID);

    nodes.fftLeft = new (std::nothrow) CFFT<cf32,512,cf32,512>(*(fifos.fifo5),*(fifos.fifo6));
    if (nodes.fftLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMFFTLEFT_ID]=createStreamNode(*nodes.fftLeft);
    nodes.fftLeft->setID(STREAMFFTLEFT_ID);

    nodes.nullSinkRight = new (std::nothrow) NullSink<float,256>(*(fifos.fifo7));
    if (nodes.nullSinkRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMNULLSINKRIGHT_ID]=createStreamNode(*nodes.nullSinkRight);
    nodes.nullSinkRight->setID(STREAMNULLSINKRIGHT_ID);

    nodes.spectrogramLeft = new (std::nothrow) Spectrogram<cf32,512>(*(fifos.fifo6));
    if (nodes.spectrogramLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMSPECTROGRAMLEFT_ID]=createStreamNode(*nodes.spectrogramLeft);
    nodes.spectrogramLeft->setID(STREAMSPECTROGRAMLEFT_ID);

    nodes.toComplexLeft = new (std::nothrow) RealToComplex<float,512,cf32,512>(*(fifos.fifo4),*(fifos.fifo5));
    if (nodes.toComplexLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMTOCOMPLEXLEFT_ID]=createStreamNode(*nodes.toComplexLeft);
    nodes.toComplexLeft->setID(STREAMTOCOMPLEXLEFT_ID);

    nodes.to_f32 = new (std::nothrow) Convert<sq15,256,sf32,256>(*(fifos.fifo0),*(fifos.fifo1));
    if (nodes.to_f32==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMTO_F32_ID]=createStreamNode(*nodes.to_f32);
    nodes.to_f32->setID(STREAMTO_F32_ID);

    nodes.winLeft = new (std::nothrow) Hanning<float,512,float,512>(*(fifos.fifo3),*(fifos.fifo4));
    if (nodes.winLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMWINLEFT_ID]=createStreamNode(*nodes.winLeft);
    nodes.winLeft->setID(STREAMWINLEFT_ID);

    nodes.display = new (std::nothrow) AppDisplay;
    if (nodes.display==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAMDISPLAY_ID]=createStreamNode(*nodes.display);
    nodes.display->setID(STREAMDISPLAY_ID);


/* Subscribe nodes for the event system*/
    nodes.spectrogramLeft->subscribe(0,*nodes.display,0);

    initError = CG_SUCCESS;
    initError = nodes.audio->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.audioWinLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.deinterleave->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.fftLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.nullSinkRight->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.spectrogramLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.toComplexLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.to_f32->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.winLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.display->init();
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
    if (fifos.fifo7!=NULL)
    {
       delete fifos.fifo7;
    }

    if (nodes.audio!=NULL)
    {
        delete nodes.audio;
    }
    if (nodes.audioWinLeft!=NULL)
    {
        delete nodes.audioWinLeft;
    }
    if (nodes.deinterleave!=NULL)
    {
        delete nodes.deinterleave;
    }
    if (nodes.fftLeft!=NULL)
    {
        delete nodes.fftLeft;
    }
    if (nodes.nullSinkRight!=NULL)
    {
        delete nodes.nullSinkRight;
    }
    if (nodes.spectrogramLeft!=NULL)
    {
        delete nodes.spectrogramLeft;
    }
    if (nodes.toComplexLeft!=NULL)
    {
        delete nodes.toComplexLeft;
    }
    if (nodes.to_f32!=NULL)
    {
        delete nodes.to_f32;
    }
    if (nodes.winLeft!=NULL)
    {
        delete nodes.winLeft;
    }
    if (nodes.display!=NULL)
    {
        delete nodes.display;
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
        for(; id < 9; id++)
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
                    
                   cgStaticError = nodes.audioWinLeft->run();
                }
                break;

                case 2:
                {
                    
                   cgStaticError = nodes.deinterleave->run();
                }
                break;

                case 3:
                {
                    
                   cgStaticError = nodes.fftLeft->run();
                }
                break;

                case 4:
                {
                    
                   cgStaticError = nodes.nullSinkRight->run();
                }
                break;

                case 5:
                {
                    
                   cgStaticError = nodes.spectrogramLeft->run();
                }
                break;

                case 6:
                {
                    
                   cgStaticError = nodes.toComplexLeft->run();
                }
                break;

                case 7:
                {
                    
                   cgStaticError = nodes.to_f32->run();
                }
                break;

                case 8:
                {
                    
                   cgStaticError = nodes.winLeft->run();
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
