#pragma once


#include <utility>
#include <variant>

#include "nodes/ZephyrLCD.hpp"

using namespace arm_cmsis_stream;

class AppDisplay : public ZephyrLCD
{
  public:
    AppDisplay() : ZephyrLCD()
    {
    }

    cg_status init() final override
    {
        drawFrame();

        return ZephyrLCD::init();
    }

    virtual ~AppDisplay() {};

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

    void strokeRectangle(int x, int y, int width, int height, uint16_t color)
    {
        uint16_t *renderingFrame = (uint16_t *)this->renderingFrame();
        bool drawTop = true;
        bool drawBottom = true;
        bool drawLeft = true;
        bool drawRight = true;
        if (x < 0)
        {
            width += x;
            x = 0;
            drawLeft = false;
        }
        if (y < 0)
        {
            height += y;
            y = 0;
            drawTop = false;
        }
        if (width + x > DISPLAY_WIDTH)
        {
            width = DISPLAY_WIDTH - x;
            drawRight = false;
        }
        if (height + y > DISPLAY_HEIGHT)
        {
            height = DISPLAY_HEIGHT - y;
            drawBottom = false;
        }

        if (width <= 0)
            return;
        if (height <= 0)
            return;
        if (drawTop)
        {
            for (int j = 0; j < width; j++)
            {
                int px = x + j;
                int py = y;
                renderingFrame[py * DISPLAY_WIDTH + px] = color;
            }
        }
        if (drawBottom)
        {
            for (int j = 0; j < width; j++)
            {
                int px = x + j;
                int py = y + height - 1;
                renderingFrame[py * DISPLAY_WIDTH + px] = color;
            }
        }
        if (drawLeft)
        {
            for (int i = 0; i < height; i++)
            {
                int px = x;
                int py = y + i;
                renderingFrame[py * DISPLAY_WIDTH + px] = color;
            }
        }
        if (drawRight)
        {
            for (int i = 0; i < height; i++)
            {
                int px = x + width - 1;
                int py = y + i;
                renderingFrame[py * DISPLAY_WIDTH + px] = color;
            }
        }
    }

    void drawSpectrogram(int pos, const TensorPtr<float> &s)
    {
        bool lockError;
        s.lock_shared(lockError, [this, pos](const Tensor<float> &tensor)
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
                        fillRectangle(pos,
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
        memset(renderingFrame, 0x00, DISPLAY_IMAGE_SIZE);

        // fillRectangle(0,0,CAMERA_FRAME_WIDTH,CAMERA_FRAME_HEIGHT,0x03F << 5);
        // Draw spectrograms

        drawSpectrogram(PADDING_LEFT, leftSpectrogram);
        drawSpectrogram(PADDING_LEFT + boxWidth + HORIZONTAL_SEPARATION, rightSpectrogram);

        strokeRectangle(PADDING_LEFT, PADDING_TOP, boxWidth, boxHeight, 0x00);
        strokeRectangle(PADDING_LEFT + boxWidth + HORIZONTAL_SEPARATION, PADDING_TOP, boxWidth, boxHeight, 0x00);

    
    }

    void processEvent(int dstPort, Event &&evt) final override
    {

        // New camera frame or spectrogram
        if (evt.event_id == kValue)
        {
           

            // New left spectrogram
            if (dstPort == 0)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply<TensorPtr<float>>(&AppDisplay::processLeftSpectrogram, *this);
                    return;
                }
            }

            // New right spectrogram
            if (dstPort == 1)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply<TensorPtr<float>>(&AppDisplay::processRightSpectrogram, *this);
                    return;
                }
            }
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

    TensorPtr<float> leftSpectrogram;
    TensorPtr<float> rightSpectrogram;
};