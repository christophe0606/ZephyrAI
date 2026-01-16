#pragma once

#include "nodes/ZephyrLCD.hpp"
#include "appnodes/ImgUtils.hpp"

using namespace arm_cmsis_stream;

class CameraFrame : public ZephyrLCD

{
    static constexpr uint16_t refresh = 40; // ms

   
      public:
	CameraFrame() : ZephyrLCD()
	{
	}

	cg_status init() final override
	{
		cg_status err = ZephyrLCD::init();
        if (err != CG_SUCCESS) {
            return err;
        }

        //Event evt(kDo, kNormalPriority);
        //evt.setTTL(refresh);
        //EventQueue::cg_eventQueue->push(LocalDestination{this, 0}, std::move(evt));
  
		return CG_SUCCESS;
	}

	virtual ~CameraFrame() {};
	

    
    void drawImage(const uint16_t *renderingFrame)
    {
        bool lockError;
        if (!image) {
            return;
        }
        image.lock_shared(lockError, [this,renderingFrame](const Tensor<const uint16_t> &tensor)
        {
           
            const uint16_t *buf = tensor.buffer();
            if (buf)
            {
                uint16_t *renderingFrame = (uint16_t *)this->renderingFrame();
                ::displayImage(renderingFrame, buf, tensor.dims[1], tensor.dims[0]);
            }
        });
    }

    void drawFrame() final override
    {
        uint16_t *renderingFrame = (uint16_t *)this->renderingFrame();
        if (renderingFrame == nullptr)
        {
            LOG_ERR("Failed to get rendering frame");
            return;
        }

        memset(renderingFrame, 0x00, DISPLAY_IMAGE_SIZE);


        drawImage(renderingFrame);


        
    }

    void genNewFrame()
    {
        // generate a new frame
        bool canRender = this->renderNewFrame();
        (void)canRender;
        // Ask for a new frame to be rendered
        //Event evt(kDo, kNormalPriority);
        //evt.setTTL(refresh);
        //EventQueue::cg_eventQueue->push(LocalDestination{this, 0}, std::move(evt));
    }

    void processEvent(int dstPort, Event &&evt) final override
    {
        if (evt.event_id == kValue)
        {
            if (dstPort == 0)
            {
                if (evt.wellFormed<TensorPtr<const uint16_t>>())
                {
                    evt.apply<TensorPtr<const uint16_t>>(&CameraFrame::processImage, *this);
                    genNewFrame();
                }
            }
        }

        
    }
protected:
void processImage(TensorPtr<const uint16_t> &&frame)
    {
        image = std::move(frame);
    }

   TensorPtr<const uint16_t> image;
};
