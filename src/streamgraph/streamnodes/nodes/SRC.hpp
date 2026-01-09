#pragma once

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"

#include "dsp/support_functions.h"
#include "dsp/filtering_functions.h"

using namespace arm_cmsis_stream;

template <typename IN, int inputSize,
          typename OUT, int outputSize>
class SRC;

template <int inputSamples, int outputSamples>
class SRC<float, inputSamples, float, outputSamples> : public GenericNode<float, inputSamples, float, outputSamples>
{
    static constexpr float coefs[48] = {-0.00054287f, -0.00119217f, -0.00071459f, 0.00090625f, 0.00235874f,
                                        0.00154289f, -0.00200666f, -0.00516361f, -0.00328125f, 0.00412056f,
                                        0.01023812f, 0.0063016f, -0.00770119f, -0.01872641f, -0.0113522f,
                                        0.01376356f, 0.03348897f, 0.02053425f, -0.02554846f, -0.06515091f,
                                        -0.04330953f, 0.06216354f, 0.21066367f, 0.3186077f, 0.3186077f,
                                        0.21066367f, 0.06216354f, -0.04330953f, -0.06515091f, -0.02554846f,
                                        0.02053425f, 0.03348897f, 0.01376356f, -0.0113522f, -0.01872641f,
                                        -0.00770119f, 0.0063016f, 0.01023812f, 0.00412056f, -0.00328125f,
                                        -0.00516361f, -0.00200666f, 0.00154289f, 0.00235874f, 0.00090625f,
                                        -0.00071459f, -0.00119217f, -0.00054287};

    static constexpr uint32_t L = 3;               // interpolation factor
    static constexpr uint32_t TAPS_PER_PHASE = 16; // adjust for quality/CPU
    static constexpr uint32_t NUM_TAPS = (L * TAPS_PER_PHASE);

  public:
    static_assert(outputSamples == 3 * inputSamples, "SRC implementation only supports 16kHz to 48kHz conversion");
    SRC(FIFOBase<float> &src, FIFOBase<float> &dst)
        : GenericNode<float, inputSamples, float, outputSamples>(src, dst) {
            arm_fir_interpolate_init_f32(&S,
                                 L,
                                 NUM_TAPS,
                                 coefs,
                                 state,
                                 inputSamples);
        };


    int run() final
    {
        float *out48 = this->getWriteBuffer();
        float *in16 = this->getReadBuffer();

        arm_fir_interpolate_f32(&S, in16, out48, inputSamples);

        return (CG_SUCCESS);
    };
protected:
    arm_fir_interpolate_instance_f32  S;
    float state[TAPS_PER_PHASE + inputSamples - 1];
};
