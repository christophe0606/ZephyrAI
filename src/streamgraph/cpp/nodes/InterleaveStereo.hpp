#pragma once

#include "arm_stream_custom_config.hpp"
#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"


using namespace arm_cmsis_stream;

template <typename IN1, int inputSize1,
          typename IN2, int inputSize2,
          typename OUT2, int outputSize2>
class InterleaveStereo;

template <int inputSamples>
class InterleaveStereo<float32_t, inputSamples, float32_t, inputSamples, sf32, inputSamples> : public GenericNode21<float32_t, inputSamples, float32_t, inputSamples, sf32, inputSamples>
{

  public:
    InterleaveStereo(FIFOBase<float32_t> &left, FIFOBase<float32_t> &right, FIFOBase<sf32> &dst)
        : GenericNode21<float32_t, inputSamples, float32_t, inputSamples, sf32, inputSamples>(left, right,dst) {};

    int prepareForRunning() final
    {
        if ((this->willOverflow()) || (this->willUnderflow1()) || (this->willUnderflow2()))
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {
        sf32 *o = this->getWriteBuffer();
        float32_t *l = this->getReadBuffer1();
        float32_t *r = this->getReadBuffer2();
        for (int i = 0; i < inputSamples; i++)
        {
            o[i].left = l[i];
            o[i].right = r[i];
        }

        return (CG_SUCCESS);
    };
};