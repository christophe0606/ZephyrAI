#include <atomic>
#include "custom.hpp"
#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"

extern "C" 
{
   #include "dbuf_display/display.h"
}

using namespace arm_cmsis_stream;

class ZephyrLCD : public StreamNode
{
    public:
	ZephyrLCD() : StreamNode()
	{
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

	cg_status init() override
	{
        int err = display_init();
        if (err != 0) {
            return CG_INIT_FAILURE;
        }

		drawFrame();
		display_next_frame();
		return CG_SUCCESS;
	}

      protected:
	std::atomic<bool> inRender{false};
};