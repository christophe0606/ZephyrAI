#include "nodes/ZephyrLCD.hpp"

using namespace arm_cmsis_stream;

class DebugDisplay : public ZephyrLCD

{
    static constexpr uint16_t redColor = 0x01F << 11;

      public:
	DebugDisplay() : ZephyrLCD()
	{
	}

	cg_status init() final override
	{
		cg_status err = ZephyrLCD::init();
        if (err != CG_SUCCESS) {
            return err;
        }
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
        fillRectangle(10, 10, 100, 50, redColor); // Red rectangle
        
        
    }

    void genNewFrame()
    {
        // generate a new frame
        bool canRender = this->renderNewFrame();
        (void)canRender;
        // Ask for a new frame to be rendered
        Event evt(kDo, kNormalPriority);
        evt.setTTL(40);
        EventQueue::cg_eventQueue->push(LocalDestination{this, 0}, std::move(evt));
    }

    void processEvent(int dstPort, Event &&evt) final override
    {
        //printf("KWS Display: event %d\n", evt.event_id);
        if (evt.event_id == kDo)
        {
            genNewFrame();
        }

        
    }
};
