#pragma once

#include "cg_enums.h"
#include "custom.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"

using namespace arm_cmsis_stream;

template <typename IN, int inputSize,
          typename OUT1, int outputSize1,
          typename OUT2, int outputSize2>
class DeinterleaveStereo;

template <int inputSamples>
class DeinterleaveStereo<sf32, inputSamples, float32_t, inputSamples, float32_t, inputSamples> : public GenericNode12<sf32, inputSamples, float32_t, inputSamples, float32_t, inputSamples>
{

  public:
    DeinterleaveStereo(FIFOBase<sf32> &src, FIFOBase<float32_t> &left, FIFOBase<float32_t> &right)
        : GenericNode12<sf32, inputSamples, float32_t, inputSamples, float32_t, inputSamples>(src, left, right) {};

    int prepareForRunning() final
    {
        if ((this->willOverflow1()) || (this->willOverflow2()) || (this->willUnderflow()))
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {
        float32_t *l = this->getWriteBuffer1();
        float32_t *r = this->getWriteBuffer2();
        sf32 *in = this->getReadBuffer();
        for (int i = 0; i < inputSamples; i++)
        {
            l[i] = in[i].left;
            r[i] = in[i].right;
        }

        return (CG_SUCCESS);
    };
};

template <int inputSamples>
class DeinterleaveStereo<sq15, inputSamples, q15_t, inputSamples, q15_t, inputSamples> : public GenericNode12<sq15, inputSamples, q15_t, inputSamples, q15_t, inputSamples>
{

  public:
    DeinterleaveStereo(FIFOBase<sq15> &src, FIFOBase<q15_t> &left, FIFOBase<q15_t> &right)
        : GenericNode12<sq15, inputSamples, q15_t, inputSamples, q15_t, inputSamples>(src, left, right) {};

    int prepareForRunning() final
    {
        if ((this->willOverflow1()) || (this->willOverflow2()) || (this->willUnderflow()))
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {
        q15_t *l = this->getWriteBuffer1();
        q15_t *r = this->getWriteBuffer2();
        sq15 *in = this->getReadBuffer();
        for (int i = 0; i < inputSamples; i++)
        {
            l[i] = in[i].left;
            r[i] = in[i].right;
        }

        return (CG_SUCCESS);
    };
};