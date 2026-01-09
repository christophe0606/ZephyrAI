#pragma once

#include "nodes/ZephyrLCD.hpp"
#include "appnodes/ImgUtils.hpp"

using namespace arm_cmsis_stream;

class SpectrogramDisplay : public ZephyrLCD

{
    static constexpr uint16_t refresh = 40; // ms

    static constexpr int PADDING_LEFT = 10;
    static constexpr int PADDING_RIGHT = 10;
    static constexpr int PADDING_TOP = 10;
    static constexpr int PADDING_BOTTOM = 10;
    static constexpr int HORIZONTAL_SEPARATION = 10;
    static constexpr int boxWidth = (DISPLAY_WIDTH - PADDING_LEFT - PADDING_RIGHT - HORIZONTAL_SEPARATION) / 2;
    static constexpr int boxHeight = DISPLAY_HEIGHT - PADDING_TOP - PADDING_BOTTOM;
    static constexpr int delta = (int)(boxHeight / (float)(CONFIG_NB_BINS - 1));
    static constexpr uint16_t redColor = 0x01F << 11;
    static constexpr uint16_t greenColor = 0x03F << 5;
    static constexpr uint16_t orangeColor = redColor | (0x00F << 5);

      public:
	SpectrogramDisplay() : ZephyrLCD()
	{
	}

	cg_status init() final override
	{
        last_ms_ = k_cyc_to_ms_near32(CG_GET_TIME_STAMP());
		cg_status err = ZephyrLCD::init();
        if (err != CG_SUCCESS) {
            return err;
        }

        //Event evt(kDo, kNormalPriority);
        //evt.setTTL(refresh);
        //EventQueue::cg_eventQueue->push(LocalDestination{this, 0}, std::move(evt));
  
		return CG_SUCCESS;
	}

	virtual ~SpectrogramDisplay() {};
	

    
    void drawSpectrogram(uint16_t *renderingFrame,int pos, const TensorPtr<float> &s)
    {
        bool lockError;
        s.lock_shared(lockError, [this, pos,renderingFrame](const Tensor<float> &tensor)
        {
           
                if (tensor.dims[0] == CONFIG_NB_BINS)
                {
                    const float *buf = tensor.buffer();
                    float p = 0;

                    for (int i = 0; i < CONFIG_NB_BINS; i++)
                    {
                        p = i * delta;
           
                        float v = buf[i];
                        if (v > 1.0f)
                            v = 1.0f;
                        if (v < 0.0f)
                            v = 0.0f;
                        fillRectangle(renderingFrame, pos,
                                      (int)(PADDING_TOP + p),
                                      (int)(boxWidth * v),
                                      delta,
                                      greenColor);
                }   
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


        drawSpectrogram(renderingFrame,PADDING_LEFT, leftSpectrogram);
        drawSpectrogram(renderingFrame,PADDING_LEFT + boxWidth + HORIZONTAL_SEPARATION, rightSpectrogram);


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
        color = color << 5; // green channel
        // For debugging.
        #if 0
        if (color != 0)
           fillRectangle(renderingFrame,10, 10, 100, 50, color); // Green rectangle
        #endif
        strokeRectangle(renderingFrame,PADDING_LEFT, PADDING_TOP, boxWidth, boxHeight, 0xFFFF);
        strokeRectangle(renderingFrame,PADDING_LEFT + boxWidth + HORIZONTAL_SEPARATION, PADDING_TOP, boxWidth, boxHeight, 0xFFFF);

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
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply<TensorPtr<float>>(&SpectrogramDisplay::processLeftSpectrogram, *this);
                }
            }

            if (dstPort == 1)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply<TensorPtr<float>>(&SpectrogramDisplay::processRightSpectrogram, *this);
                }
            }
            genNewFrame();
        }

        
    }
protected:
void processLeftSpectrogram(TensorPtr<float> &&frame)
    {
        leftSpectrogram = std::move(frame);
    }

    void processRightSpectrogram(TensorPtr<float> &&frame)
    {
        rightSpectrogram = std::move(frame);
    }

   int period_ms_ = 1000;
   float alpha = 1.0f;
   uint32_t last_ms_=0;
   TensorPtr<float> leftSpectrogram;
   TensorPtr<float> rightSpectrogram;
};
