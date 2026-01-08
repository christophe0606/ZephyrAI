#pragma once

#include "nodes/ZephyrLCD.hpp"

using namespace arm_cmsis_stream;

class DebugDisplay : public ZephyrLCD

{
    static constexpr uint16_t redColor = 0x01F << 11;
    static constexpr uint16_t refresh = 40; // ms

      public:
	DebugDisplay() : ZephyrLCD()
	{
	}

	cg_status init() final override
	{
        last_ms_ = k_cyc_to_ms_near32(CG_GET_TIME_STAMP());
		cg_status err = ZephyrLCD::init();
        if (err != CG_SUCCESS) {
            return err;
        }

        Event evt(kDo, kNormalPriority);
        evt.setTTL(refresh);
        EventQueue::cg_eventQueue->push(LocalDestination{this, 0}, std::move(evt));
  
		return CG_SUCCESS;
	}

	virtual ~DebugDisplay() {};
	

    void fillRectangle(int x, int y, int width, int height, uint16_t color)
    {
        uint16_t *renderingFrame = (uint16_t *)this->renderingFrame();
        if (x < 0)
        {
            width += x;
            x = 0;
        }
        if (y < 0)
        {
            height += y;
            y = 0;
        }
        if (width + x > DISPLAY_WIDTH)
        {
            width = DISPLAY_WIDTH - x;
        }
        if (height + y > DISPLAY_HEIGHT)
        {
            height = DISPLAY_HEIGHT - y;
        }
        if (width <= 0)
            return;
        if (height <= 0)
            return;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                int px = x + j;
                int py = y + i;
                renderingFrame[py * DISPLAY_WIDTH + px] = color;
            }
        }
    }
   

    void drawFrame() final override
    {
        uint16_t *renderingFrame = (uint16_t *)this->renderingFrame();
        if (renderingFrame == nullptr)
        {
            LOG_ERR("Failed to get rendering frame");
            return;
        }

        /* draw something */
        uint32_t current_ms = k_cyc_to_ms_near32(CG_GET_TIME_STAMP());
        float delta = 1.0f - float(current_ms - last_ms_)/period_ms_;
        if (delta < 0.0f)
        {
            delta = 0.0f;
            last_ms_ = current_ms;
        }
        uint16_t color = uint16_t(0x1F * delta);
        if (color > 0x1F)
        {
            color = 0x1F;
        }
        color = color << 11; // Red channel
        fillRectangle(10, 10, 100, 50, color); // Red rectangle
        
        
    }

    void genNewFrame()
    {
        // generate a new frame
        bool canRender = this->renderNewFrame();
        (void)canRender;
        // Ask for a new frame to be rendered
        Event evt(kDo, kNormalPriority);
        evt.setTTL(refresh);
        EventQueue::cg_eventQueue->push(LocalDestination{this, 0}, std::move(evt));
    }

    void processEvent(int dstPort, Event &&evt) final override
    {
        //LOG_INF("Debug Display: event %d\n", evt.event_id);
        if (evt.event_id == kDo)
        {
            genNewFrame();
        }

        
    }
protected:
   int period_ms_ = 1000;
   float alpha = 1.0f;
   uint32_t last_ms_=0;
};
