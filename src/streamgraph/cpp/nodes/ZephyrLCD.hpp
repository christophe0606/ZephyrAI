#include <atomic>
#include "arm_stream_custom_config.hpp"
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"

extern "C" 
{
   #include "dbuf_display/display.h"
}

#define DISPLAY_IMAGE_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t))

using namespace arm_cmsis_stream;

class ZephyrLCD : public StreamNode
{
    public:
	ZephyrLCD() : StreamNode()
	{
	};

    cg_status init() override
	{
		void *buf = display_active_buffer();
		memset(buf,0,DISPLAY_IMAGE_SIZE);

		buf = display_inactive_buffer();
		memset(buf,0,DISPLAY_IMAGE_SIZE);

		int err = display_init();
        if (err != 0) {
            return CG_INIT_FAILURE;
        }

		this->drawFrame();
		display_next_frame();

		return CG_SUCCESS;
	};


	void *renderingFrame() const
	{
		return display_inactive_buffer();
	};

	virtual ~ZephyrLCD()
	{
	};

	virtual void drawFrame() = 0;

	// The node was asked to render a new frame
	bool renderNewFrame()
	{
		if (inRender.load()) {
			LOG_DBG("Already in render\n");
			return false;
		}

		inRender.store(true);

		this->drawFrame();

		inRender.store(false);

        display_next_frame();
		return true;
	}

	
      protected:
	std::atomic<bool> inRender{false};
};