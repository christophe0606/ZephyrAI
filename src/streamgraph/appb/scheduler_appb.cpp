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
#include "AppNodes_appb.hpp"
#include "scheduler_appb.h"
#include "appb_extern_templates.hpp"

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
static uint8_t schedule[14]=
{ 
0,6,11,3,1,12,9,4,7,2,13,10,5,8,
};

/*

Internal ID identification for the nodes

*/
#define AUDIO_INTERNAL_ID 0
#define AUDIOWINLEFT_INTERNAL_ID 1
#define AUDIOWINRIGHT_INTERNAL_ID 2
#define DEINTERLEAVE_INTERNAL_ID 3
#define FFTLEFT_INTERNAL_ID 4
#define FFTRIGHT_INTERNAL_ID 5
#define GAIN_INTERNAL_ID 6
#define SPECTROGRAMLEFT_INTERNAL_ID 7
#define SPECTROGRAMRIGHT_INTERNAL_ID 8
#define TOCOMPLEXLEFT_INTERNAL_ID 9
#define TOCOMPLEXRIGHT_INTERNAL_ID 10
#define TO_F32_INTERNAL_ID 11
#define WINLEFT_INTERNAL_ID 12
#define WINRIGHT_INTERNAL_ID 13
#define DISPLAY_INTERNAL_ID 14



/***********

Node identification

************/
static CStreamNode identifiedNodes[STREAM_APPB_NB_IDENTIFIED_NODES]={0};

CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/
#define FIFOSIZE0 320
#define FIFOSIZE1 320
#define FIFOSIZE2 320
#define FIFOSIZE3 320
#define FIFOSIZE4 640
#define FIFOSIZE5 1024
#define FIFOSIZE6 1024
#define FIFOSIZE7 1024
#define FIFOSIZE8 320
#define FIFOSIZE9 640
#define FIFOSIZE10 1024
#define FIFOSIZE11 1024
#define FIFOSIZE12 1024

#define BUFFERSIZE0 8192
CG_BEFORE_BUFFER
uint8_t stream_appb_buf0[BUFFERSIZE0]={0};

#define BUFFERSIZE1 8192
CG_BEFORE_BUFFER
uint8_t stream_appb_buf1[BUFFERSIZE1]={0};

#define BUFFERSIZE2 8192
CG_BEFORE_BUFFER
uint8_t stream_appb_buf2[BUFFERSIZE2]={0};


typedef struct {
FIFO<sq15,FIFOSIZE0,1,0> *fifo0;
FIFO<sq15,FIFOSIZE1,1,0> *fifo1;
FIFO<sf32,FIFOSIZE2,1,0> *fifo2;
FIFO<float,FIFOSIZE3,1,0> *fifo3;
FIFO<float,FIFOSIZE4,1,0> *fifo4;
FIFO<float,FIFOSIZE5,1,0> *fifo5;
FIFO<cf32,FIFOSIZE6,1,0> *fifo6;
FIFO<cf32,FIFOSIZE7,1,0> *fifo7;
FIFO<float,FIFOSIZE8,1,0> *fifo8;
FIFO<float,FIFOSIZE9,1,0> *fifo9;
FIFO<float,FIFOSIZE10,1,0> *fifo10;
FIFO<cf32,FIFOSIZE11,1,0> *fifo11;
FIFO<cf32,FIFOSIZE12,1,0> *fifo12;
} fifos_t;

typedef struct {
    ZephyrAudioSource<sq15,320> *audio;
    SlidingBuffer<float,640,320> *audioWinLeft;
    SlidingBuffer<float,640,320> *audioWinRight;
    DeinterleaveStereo<sf32,320,float,320,float,320> *deinterleave;
    CFFT<cf32,1024,cf32,1024> *fftLeft;
    CFFT<cf32,1024,cf32,1024> *fftRight;
    Gain<sq15,320,sq15,320> *gain;
    Spectrogram<cf32,1024> *spectrogramLeft;
    Spectrogram<cf32,1024> *spectrogramRight;
    RealToComplex<float,1024,cf32,1024> *toComplexLeft;
    RealToComplex<float,1024,cf32,1024> *toComplexRight;
    Convert<sq15,320,sf32,320> *to_f32;
    Hanning<float,640,float,1024> *winLeft;
    Hanning<float,640,float,1024> *winRight;
    SpectrogramDisplay *display;
} nodes_t;


static fifos_t fifos={0};

static nodes_t nodes={0};

CStreamNode* get_scheduler_appb_node(int32_t nodeID)
{
    if (nodeID >= STREAM_APPB_NB_IDENTIFIED_NODES)
    {
        return(nullptr);
    }
    if (nodeID < 0)
    {
        return(nullptr);
    }
    return(&identifiedNodes[nodeID]);
}

int init_scheduler_appb(void *evtQueue_,AppbParams *params)
{
    EventQueue *evtQueue = reinterpret_cast<EventQueue *>(evtQueue_);

    CG_BEFORE_FIFO_INIT;
    fifos.fifo0 = new (std::nothrow) FIFO<sq15,FIFOSIZE0,1,0>(stream_appb_buf1);
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo1 = new (std::nothrow) FIFO<sq15,FIFOSIZE1,1,0>(stream_appb_buf0);
    if (fifos.fifo1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo2 = new (std::nothrow) FIFO<sf32,FIFOSIZE2,1,0>(stream_appb_buf1);
    if (fifos.fifo2==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo3 = new (std::nothrow) FIFO<float,FIFOSIZE3,1,0>(stream_appb_buf2);
    if (fifos.fifo3==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo4 = new (std::nothrow) FIFO<float,FIFOSIZE4,1,0>(stream_appb_buf1);
    if (fifos.fifo4==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo5 = new (std::nothrow) FIFO<float,FIFOSIZE5,1,0>(stream_appb_buf2);
    if (fifos.fifo5==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo6 = new (std::nothrow) FIFO<cf32,FIFOSIZE6,1,0>(stream_appb_buf1);
    if (fifos.fifo6==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo7 = new (std::nothrow) FIFO<cf32,FIFOSIZE7,1,0>(stream_appb_buf2);
    if (fifos.fifo7==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo8 = new (std::nothrow) FIFO<float,FIFOSIZE8,1,0>(stream_appb_buf0);
    if (fifos.fifo8==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo9 = new (std::nothrow) FIFO<float,FIFOSIZE9,1,0>(stream_appb_buf1);
    if (fifos.fifo9==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo10 = new (std::nothrow) FIFO<float,FIFOSIZE10,1,0>(stream_appb_buf0);
    if (fifos.fifo10==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo11 = new (std::nothrow) FIFO<cf32,FIFOSIZE11,1,0>(stream_appb_buf1);
    if (fifos.fifo11==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo12 = new (std::nothrow) FIFO<cf32,FIFOSIZE12,1,0>(stream_appb_buf0);
    if (fifos.fifo12==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    cg_status initError;

    nodes.audio = new (std::nothrow) ZephyrAudioSource<sq15,320>(*(fifos.fifo0),params->audio);
    if (nodes.audio==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPB_AUDIO_ID]=createStreamNode(*nodes.audio);
    nodes.audio->setID(STREAM_APPB_AUDIO_ID);

    nodes.audioWinLeft = new (std::nothrow) SlidingBuffer<float,640,320>(*(fifos.fifo3),*(fifos.fifo4));
    if (nodes.audioWinLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPB_AUDIOWINLEFT_ID]=createStreamNode(*nodes.audioWinLeft);
    nodes.audioWinLeft->setID(STREAM_APPB_AUDIOWINLEFT_ID);

    nodes.audioWinRight = new (std::nothrow) SlidingBuffer<float,640,320>(*(fifos.fifo8),*(fifos.fifo9));
    if (nodes.audioWinRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPB_AUDIOWINRIGHT_ID]=createStreamNode(*nodes.audioWinRight);
    nodes.audioWinRight->setID(STREAM_APPB_AUDIOWINRIGHT_ID);

    nodes.deinterleave = new (std::nothrow) DeinterleaveStereo<sf32,320,float,320,float,320>(*(fifos.fifo2),*(fifos.fifo3),*(fifos.fifo8));
    if (nodes.deinterleave==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.fftLeft = new (std::nothrow) CFFT<cf32,1024,cf32,1024>(*(fifos.fifo6),*(fifos.fifo7));
    if (nodes.fftLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.fftRight = new (std::nothrow) CFFT<cf32,1024,cf32,1024>(*(fifos.fifo11),*(fifos.fifo12));
    if (nodes.fftRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.gain = new (std::nothrow) Gain<sq15,320,sq15,320>(*(fifos.fifo0),*(fifos.fifo1),4);
    if (nodes.gain==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.spectrogramLeft = new (std::nothrow) Spectrogram<cf32,1024>(*(fifos.fifo7),evtQueue);
    if (nodes.spectrogramLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.spectrogramRight = new (std::nothrow) Spectrogram<cf32,1024>(*(fifos.fifo12),evtQueue);
    if (nodes.spectrogramRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.toComplexLeft = new (std::nothrow) RealToComplex<float,1024,cf32,1024>(*(fifos.fifo5),*(fifos.fifo6));
    if (nodes.toComplexLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.toComplexRight = new (std::nothrow) RealToComplex<float,1024,cf32,1024>(*(fifos.fifo10),*(fifos.fifo11));
    if (nodes.toComplexRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.to_f32 = new (std::nothrow) Convert<sq15,320,sf32,320>(*(fifos.fifo1),*(fifos.fifo2));
    if (nodes.to_f32==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.winLeft = new (std::nothrow) Hanning<float,640,float,1024>(*(fifos.fifo4),*(fifos.fifo5));
    if (nodes.winLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.winRight = new (std::nothrow) Hanning<float,640,float,1024>(*(fifos.fifo9),*(fifos.fifo10));
    if (nodes.winRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.display = new (std::nothrow) SpectrogramDisplay;
    if (nodes.display==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPB_DISPLAY_ID]=createStreamNode(*nodes.display);
    nodes.display->setID(STREAM_APPB_DISPLAY_ID);


/* Subscribe nodes for the event system*/
    nodes.spectrogramLeft->subscribe(0,*nodes.display,0);
    nodes.spectrogramRight->subscribe(0,*nodes.display,1);

    initError = CG_SUCCESS;
    initError = nodes.audio->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.audioWinLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.audioWinRight->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.deinterleave->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.fftLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.fftRight->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.gain->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.spectrogramLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.spectrogramRight->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.toComplexLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.toComplexRight->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.to_f32->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.winLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.winRight->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.display->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
   


    return(CG_SUCCESS);

}

void free_scheduler_appb()
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
    if (fifos.fifo8!=NULL)
    {
       delete fifos.fifo8;
    }
    if (fifos.fifo9!=NULL)
    {
       delete fifos.fifo9;
    }
    if (fifos.fifo10!=NULL)
    {
       delete fifos.fifo10;
    }
    if (fifos.fifo11!=NULL)
    {
       delete fifos.fifo11;
    }
    if (fifos.fifo12!=NULL)
    {
       delete fifos.fifo12;
    }

    if (nodes.audio!=NULL)
    {
        delete nodes.audio;
    }
    if (nodes.audioWinLeft!=NULL)
    {
        delete nodes.audioWinLeft;
    }
    if (nodes.audioWinRight!=NULL)
    {
        delete nodes.audioWinRight;
    }
    if (nodes.deinterleave!=NULL)
    {
        delete nodes.deinterleave;
    }
    if (nodes.fftLeft!=NULL)
    {
        delete nodes.fftLeft;
    }
    if (nodes.fftRight!=NULL)
    {
        delete nodes.fftRight;
    }
    if (nodes.gain!=NULL)
    {
        delete nodes.gain;
    }
    if (nodes.spectrogramLeft!=NULL)
    {
        delete nodes.spectrogramLeft;
    }
    if (nodes.spectrogramRight!=NULL)
    {
        delete nodes.spectrogramRight;
    }
    if (nodes.toComplexLeft!=NULL)
    {
        delete nodes.toComplexLeft;
    }
    if (nodes.toComplexRight!=NULL)
    {
        delete nodes.toComplexRight;
    }
    if (nodes.to_f32!=NULL)
    {
        delete nodes.to_f32;
    }
    if (nodes.winLeft!=NULL)
    {
        delete nodes.winLeft;
    }
    if (nodes.winRight!=NULL)
    {
        delete nodes.winRight;
    }
    if (nodes.display!=NULL)
    {
        delete nodes.display;
    }
}

void reset_fifos_scheduler_appb(int all)
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
    if (fifos.fifo7!=NULL)
    {
       fifos.fifo7->reset();
    }
    if (fifos.fifo8!=NULL)
    {
       fifos.fifo8->reset();
    }
    if (fifos.fifo9!=NULL)
    {
       fifos.fifo9->reset();
    }
    if (fifos.fifo10!=NULL)
    {
       fifos.fifo10->reset();
    }
    if (fifos.fifo11!=NULL)
    {
       fifos.fifo11->reset();
    }
    if (fifos.fifo12!=NULL)
    {
       fifos.fifo12->reset();
    }
   // Buffers are set to zero too
   if (all)
   {
       std::fill_n(stream_appb_buf0, BUFFERSIZE0, (uint8_t)0);
       std::fill_n(stream_appb_buf1, BUFFERSIZE1, (uint8_t)0);
       std::fill_n(stream_appb_buf2, BUFFERSIZE2, (uint8_t)0);
   }
}


CG_BEFORE_SCHEDULER_FUNCTION
uint32_t scheduler_appb(int *error)
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
        for(; id < 14; id++)
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
                    
                   cgStaticError = nodes.audioWinRight->run();
                }
                break;

                case 3:
                {
                    
                   cgStaticError = nodes.deinterleave->run();
                }
                break;

                case 4:
                {
                    
                   cgStaticError = nodes.fftLeft->run();
                }
                break;

                case 5:
                {
                    
                   cgStaticError = nodes.fftRight->run();
                }
                break;

                case 6:
                {
                    
                   cgStaticError = nodes.gain->run();
                }
                break;

                case 7:
                {
                    
                   cgStaticError = nodes.spectrogramLeft->run();
                }
                break;

                case 8:
                {
                    
                   cgStaticError = nodes.spectrogramRight->run();
                }
                break;

                case 9:
                {
                    
                   cgStaticError = nodes.toComplexLeft->run();
                }
                break;

                case 10:
                {
                    
                   cgStaticError = nodes.toComplexRight->run();
                }
                break;

                case 11:
                {
                    
                   cgStaticError = nodes.to_f32->run();
                }
                break;

                case 12:
                {
                    
                   cgStaticError = nodes.winLeft->run();
                }
                break;

                case 13:
                {
                    
                   cgStaticError = nodes.winRight->run();
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
