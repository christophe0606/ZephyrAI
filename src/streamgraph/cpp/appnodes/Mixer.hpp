#pragma once

#include "cg_enums.h"
#include "arm_stream_custom_config.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"

using namespace arm_cmsis_stream;

template <typename IN1, int inputSize1,
          typename IN2, int inputSize2,
          typename OUT1, int output1Size,
          typename OUT2, int output2Size>
class GenericNode22 : public NodeBase
{
  public:
    explicit GenericNode22(FIFOBase<IN1> &src1, 
                           FIFOBase<IN2> &src2,
                           FIFOBase<OUT1> &dst1, 
                           FIFOBase<OUT2> &dst2)
        : mSrc1(src1), mSrc2(src2),
          mDst1(dst1), mDst2(dst2) {};

  protected:
    OUT1 *getWriteBuffer1(int nb = output1Size)
    {
        return mDst1.getWriteBuffer(nb);
    };
    OUT2 *getWriteBuffer2(int nb = output2Size)
    {
        return mDst2.getWriteBuffer(nb);
    };
    IN1 *getReadBuffer1(int nb = inputSize1)
    {
        return mSrc1.getReadBuffer(nb);
    };
    IN2 *getReadBuffer2(int nb = inputSize2)
    {
        return mSrc2.getReadBuffer(nb);
    };

    bool willOverflow1(int nb = output1Size) const
    {
        return mDst1.willOverflowWith(nb);
    };
    bool willOverflow2(int nb = output2Size) const
    {
        return mDst2.willOverflowWith(nb);
    };

    bool willUnderflow1(int nb = inputSize1) const
    {
        return mSrc1.willUnderflowWith(nb);
    };
    bool willUnderflow2(int nb = inputSize2) const
    {
        return mSrc2.willUnderflowWith(nb);
    };

  private:
    FIFOBase<IN1> &mSrc1;
    FIFOBase<IN2> &mSrc2;
    FIFOBase<OUT1> &mDst1;
    FIFOBase<OUT2> &mDst2;
};

template <typename IN1, int inputSize1,
          typename IN2, int inputSize2,
          typename OUT1, int output1Size,
          typename OUT2, int output2Size>
class Mixer;

template <int inputSamples>
class Mixer<float, inputSamples, float, inputSamples, float, inputSamples, float, inputSamples> : public GenericNode22<float, inputSamples, float, inputSamples, float, inputSamples, float, inputSamples>
{
  public:
    explicit Mixer(FIFOBase<float> &src1,
                   FIFOBase<float> &src2,
                   FIFOBase<float> &dst1,
                   FIFOBase<float> &dst2)
        : GenericNode22<float, inputSamples, float, inputSamples, float, inputSamples, float, inputSamples>(src1, src2, dst1, dst2){
            delta = 2.0f * 3.14159265359f / SAMPLE_RATE * 2.0f;
            phase = 0.0f;
        };

    int prepareForRunning() final
    {
         if ((this->willOverflow1()) || this->willUnderflow1() || (this->willOverflow2()) || this->willUnderflow2())
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    }

     int run() final
    {

        float *inl = this->getReadBuffer1();
        float *inr = this->getReadBuffer2();
        float *outl = this->getWriteBuffer1();
        float *outr = this->getWriteBuffer2();

        for (int i = 0; i < inputSamples; i++)
        {
            float alpha = (sinf(phase) + 1.0f) / 2.0f;
            outl[i] = alpha*inl[i] + (1-alpha)*inr[i];
            outr[i] = (1-alpha)*inl[i] + inr[i];

           phase += delta;
           if (phase > 2.0f * 3.14159265359f)
               phase -= 2.0f * 3.14159265359f;
        }

        return(CG_SUCCESS);
       
    };

protected:
    float phase;
    float delta;
};

