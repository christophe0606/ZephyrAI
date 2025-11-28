#pragma once

#include "RTE_Components.h"
#include "config.h"

#include CMSIS_device_header

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "custom.hpp"

extern "C"
{
#include "cmsis_os2.h"
#include "cmsis_vstream.h"
#include "camera.h"
#include "config.h"
}

using namespace arm_cmsis_stream;

const osThreadAttr_t videoSrcAttr = {
    .stack_size = 4096,
    .priority = osPriorityHigh};

extern vDbgStreamDriver_t Driver_vDbgStreamVideoIn;
#define vStream_VideoIn (&Driver_vDbgStreamVideoIn)

#define VSTREAM_VIDEO_SOURCE_BLOCK_EVT (0x1)

class VStreamVideoSource : public StreamNode
{
  public:
    VStreamVideoSource()
        : StreamNode()
    {

        vStream_VideoIn->Initialize(VideoSrc_Event_Callback);

        /* Set Input Video buffer */
        if (vStream_VideoIn->SetBuf(CAM_Frame, sizeof(CAM_Frame), CAMERA_FRAME_SIZE) != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to set buffer for video input\n");
            PrintErrors(vStream_VideoIn->ErrorCode());
        }

        if (vStream_VideoIn->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to start video capture\n");
            PrintErrors(vStream_VideoIn->ErrorCode());
        }
    }

    ~VStreamVideoSource()
    {
        if (vStream_VideoIn->Stop() != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to stop video input\n");
            PrintErrors(vStream_VideoIn->ErrorCode());
        }

        if (vStream_VideoIn->Uninitialize() != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to uninitialize video input\n");
            PrintErrors(vStream_VideoIn->ErrorCode());
        }
    };

    void subscribe(int outputPort, StreamNode &dst, int dstPort)
    {
        ev0.subscribe(dst, dstPort);
    }

    static void release_video_frame(void *frame)
    {
        DEBUG_PRINT("Release camera frame\n");
        if (vStream_VideoIn->ReleaseBlock() != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to release video input frame\n");
            PrintErrors(vStream_VideoIn->ErrorCode());
        }
        else
        {
            vStreamStatus_t status;
            do
            {
                status = vStream_VideoIn->GetStatus();
            } while (status.active == 1U);

            if (vStream_VideoIn->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK)
            {
                ERROR_PRINT("Failed to start video capture\n");
                PrintErrors(vStream_VideoIn->ErrorCode());
            }
        }
    }

    void processEvent(int dstPort, Event &&evt) final
    {
        if (evt.event_id == kDo)
        {
            DEBUG_PRINT("kDo for video source\n");
            uint8_t *inFrame = (uint8_t *)vStream_VideoIn->GetBlock();
            if (inFrame != nullptr)
            {
                //SCB_InvalidateDCache_by_Addr(inFrame, CAMERA_FRAME_SIZE);

                DEBUG_PRINT("Send frame\n");
                UniquePtr<uint16_t> rgb_buf((uint16_t *)inFrame, release_video_frame);
                TensorPtr<uint16_t> t = TensorPtr<uint16_t>::create_with((uint16_t)2,
                                                                         cg_tensor_dims_t{CAMERA_FRAME_HEIGHT, CAMERA_FRAME_WIDTH},
                                                                         std::move(rgb_buf));

                ev0.sendSync(kHighPriority, kValue, std::move(t)); // Send the event to the subscribed nodes
            }
            else
            {
                ERROR_PRINT("No camera frame available\n");
            }
        }
    }

    EventOutput ev0;
};