#pragma once
#include <atomic>
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"

extern "C" 
{
   #include "dbuf_display/display.h"
}

#include "init_drv_src.hpp"

using namespace arm_cmsis_stream;

class ZephyrLCD : public StreamNode, public ContextSwitch
{
    public:
	static constexpr size_t DISPLAY_IMAGE_SIZE = (DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t));

	ZephyrLCD() : StreamNode()
	{
	};

	int pause() final
	{
		return 0;
	}

	int resume() final
	{
		clear_display();
		return 0;
	}

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