#pragma once

#include "nodes/ZephyrLCD.hpp"

using namespace arm_cmsis_stream;

class KWSDisplay : public ZephyrLCD
{
    
    static constexpr uint32_t duration = 2;

  public:
    KWSDisplay(): ZephyrLCD()
    {
    }

    cg_status init() final override
    {
        return ZephyrLCD::init();
    }

    virtual ~KWSDisplay() {};

    uint32_t getTime()
    {
        return k_cyc_to_ms_near32(CG_GET_TIME_STAMP());
    }

    void drawImage(uint16_t *renderingFrame, const uint8_t *img, const uint32_t w, const uint32_t h)
    {
        if (img == nullptr)
            return;
        const uint32_t wpad = (DISPLAY_FRAME_WIDTH - w) / 2;
        const uint32_t hpad = (DISPLAY_FRAME_HEIGHT - h) / 2;
        const uint8_t a = alpha >> 7;
        for (int i = 0; i < h; i++)
        {
            int j = 0;
            uint8_t *pSrc = (uint8_t *)&img[i * w];
            uint16_t *pDst = (uint16_t *)&renderingFrame[(hpad + i) * DISPLAY_FRAME_WIDTH + wpad];
            
            for (; j <= (w-16); j+=16)
            {
                uint8x16_t src = vld1q_u8(pSrc);
                pSrc += 16;
                uint8x16_t v = vmulhq_u8(src, vdupq_n_u8(a));
                uint16x8_t l = vmovlbq_u8(v);
                uint16x8_t h = vmovltq_u8(v);

                uint16x8_t r,g,b;
                b =             vshrq_n_u16(l, 3);
                g = vshlq_n_u16(vshrq_n_u16(l, 2), 5);
                r = vshlq_n_u16(vshrq_n_u16(l, 3), 11);
                uint16x8x2_t pixel;
                pixel.val[0] = vorrq_u16(r, vorrq_u16(g, b));

                b =             vshrq_n_u16(h, 3);
                g = vshlq_n_u16(vshrq_n_u16(h, 2), 5);
                r = vshlq_n_u16(vshrq_n_u16(h, 3), 11);
                pixel.val[1] = vorrq_u16(r, vorrq_u16(g, b));
                
                
                vst2q_u16(pDst, pixel);
                pDst += 16;
                
            }
            
            for (; j < w; j++)
            {
                uint8_t o = *pSrc++;
                uint16_t v = (uint16_t)__USAT(((uint32_t)o * (uint32_t)a) >> 8, 8);
                q15_t pixel = ((v >> 3) << 11) | ((v >> 2) << 5) | (v >> 3);
                *pDst++ = pixel;
            }
        }
    }

    void drawFrame() final override
    {
        uint16_t *renderingFrame = (uint16_t *)this->renderingFrame();
        if (renderingFrame == nullptr)
        {
            ERROR_PRINT("Failed to get rendering frame");
            return;
        }
        memset(renderingFrame, 0x00, DISPLAY_IMAGE_SIZE);
        if (currentImg)
           drawImage(renderingFrame, currentImg, width, height);
    }

    void newValue(uint32_t i)
    {
        if (i >= 0 && i < 10)
        {
            currentImg = kws_imgs[i];
            width = kws_widths[i];
            height = kws_heights[i];
            startMs = getTime();
            alpha = 0x7FFF;
            displayLast = true;
            bool canRender = this->renderNewFrame();
            // Ask for new frame
            Event evt(kDo, kNormalPriority);
            EventQueue::cg_eventQueue->push(LocalDestination{this, 0}, std::move(evt));
        }
        else
        {
            currentImg = nullptr;
        }
    }

    void genNewFrame()
    {
        uint32_t currentMs = getTime();
        uint32_t delta = (0x7FFF * (currentMs - startMs)/1000/duration);
        alpha = 0;
        if ((delta <= 0x7FFF) || displayLast)
        {
            if (delta > 0x7FFF)
            {
                displayLast = false;
                alpha = 0;
            }
            else 
            {
                alpha = 0x7FFF - delta;
            }
            // generate a new frame
            bool canRender = this->renderNewFrame();
            Event evt(kDo, kNormalPriority);
            evt.setTTL(40);
            EventQueue::cg_eventQueue->push(LocalDestination{this, 0}, std::move(evt));
        }
    }

    void processEvent(int dstPort, Event &&evt) final override
    {
        //printf("KWS Display: event %d\n", evt.event_id);
        if (evt.event_id == kDo)
        {
            genNewFrame();
        }

        // New camera frame or spectrogram
        if (evt.event_id == kValue)
        {
            if (evt.wellFormed<uint32_t>())
            {
                evt.apply<uint32_t>(&KWSDisplay::newValue, *this);
            }
        }
    }

  protected:
    uint32_t startMs;
    q15_t alpha{0};
    const uint8_t *currentImg{nullptr};
    uint32_t width,height;
    bool displayLast{false};
};