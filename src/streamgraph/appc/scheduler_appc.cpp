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
#include "AppNodes_appc.hpp"
#include "scheduler_appc.h"
#include "appc_extern_templates.hpp"

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

/*

Internal ID identification for the nodes

*/
#define LCD_INTERNAL_ID 0
#define VIDEO_INTERNAL_ID 1



/***********

Node identification

************/
static CStreamNode identifiedNodes[STREAM_APPC_NB_IDENTIFIED_NODES]={0};

CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/



typedef struct {
    CameraFrame *lcd;
    ZephyrDebugVideoSource *video;
} nodes_t;



static nodes_t nodes={0};

CStreamNode* get_scheduler_appc_node(int32_t nodeID)
{
    if (nodeID >= STREAM_APPC_NB_IDENTIFIED_NODES)
    {
        return(nullptr);
    }
    if (nodeID < 0)
    {
        return(nullptr);
    }
    return(&identifiedNodes[nodeID]);
}

int init_scheduler_appc(void *evtQueue_,AppcParams *params)
{
    EventQueue *evtQueue = reinterpret_cast<EventQueue *>(evtQueue_);

    CG_BEFORE_FIFO_INIT;

    CG_BEFORE_NODE_INIT;
    cg_status initError;

    nodes.lcd = new (std::nothrow) CameraFrame;
    if (nodes.lcd==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPC_LCD_ID]=createStreamNode(*nodes.lcd);
    nodes.lcd->setID(STREAM_APPC_LCD_ID);

    nodes.video = new (std::nothrow) ZephyrDebugVideoSource(evtQueue);
    if (nodes.video==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPC_VIDEO_ID]=createStreamNode(*nodes.video);
    nodes.video->setID(STREAM_APPC_VIDEO_ID);


/* Subscribe nodes for the event system*/
    nodes.video->subscribe(0,*nodes.lcd,0);

    initError = CG_SUCCESS;
    initError = nodes.lcd->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.video->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
   


    return(CG_SUCCESS);

}

void free_scheduler_appc()
{

    if (nodes.lcd!=NULL)
    {
        delete nodes.lcd;
    }
    if (nodes.video!=NULL)
    {
        delete nodes.video;
    }
}

void reset_fifos_scheduler_appc(int all)
{
   // Buffers are set to zero too
   if (all)
   {
   }
}


CG_BEFORE_SCHEDULER_FUNCTION
uint32_t scheduler_appc(int *error)
{
        *error=CG_STOP_SCHEDULER;
#if !defined(CG_EVENTS_MULTI_THREAD)
    while(1){
        // To have possibility to process the event queue
        CG_BEFORE_ITERATION;
    };
#endif
    return(0);
}
