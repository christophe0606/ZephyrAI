#pragma once

#include "RTE_Components.h"
#include "config.h"
#include "m-profile/armv7m_cachel1.h"
#include "soc.h"
#include <utility>
#include <variant>

#include CMSIS_device_header

#include <atomic>

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "custom.hpp"

extern "C"
{
#include "lcd.h"
#include "config.h"
}

using namespace arm_cmsis_stream;

extern displayDriver_t Driver_display;
#define lcd (&Driver_display)

class VStreamVideoSink : public StreamNode
{
    static void VideoSink_Event_Callback(uint32_t event)
    {
        if (event & DISPLAY_EVENT_NEW_FRAME)
        {

            
        }

    }

  public:
    VStreamVideoSink()
        : StreamNode()
    {


        if (lcd->Initialize(VStreamVideoSink::VideoSink_Event_Callback) != DISPLAY_OK)
        {
            ERROR_PRINT("Failed to initialize LCD output\n");
        }

        /* Set Input Video buffer */
        if (lcd->SetBuf(LCD_Frame, DISPLAY_IMAGE_SIZE) != DISPLAY_OK)
        {
            ERROR_PRINT("Failed to set buffer for video output\n");
        }
    }

    void *renderingFrame() const
    {
        return lcd->GetRenderingBlock();
    }

    virtual ~VStreamVideoSink()
    {
        if (lcd->Stop() != DISPLAY_OK)
        {
            ERROR_PRINT("Failed to stop video output\n");
        }

        if (lcd->Uninitialize() != DISPLAY_OK)
        {
            ERROR_PRINT("Failed to uninitialize video output\n");
        }
    };

    virtual void drawFrame() = 0;

    // The node was asked to render a new frame
    bool
    renderNewFrame()
    {

        if (inRender.load())
        {
            DEBUG_PRINT("Already in render\n");
            return false;
        }

        inRender.store(true);

        this->drawFrame();
        
        inRender.store(false);

        displayStatus_t status;
        do
        {
            status = lcd->GetStatus();
        } while (status.active == 1U);

        lcd->SwitchBuffers();

        if (lcd->Start() != DISPLAY_OK)
        {
            ERROR_PRINT("Failed to start LCD output\n");
        }
        return true;
    }

    cg_status init() override
    {
        drawFrame();
        lcd->SwitchBuffers();
        if (lcd->Start() != DISPLAY_OK)
        {
            ERROR_PRINT("Failed to start LCD output\n");
        }
        return CG_SUCCESS;
    }

   
    
  protected:
    std::atomic<bool> inRender{false};

    
};