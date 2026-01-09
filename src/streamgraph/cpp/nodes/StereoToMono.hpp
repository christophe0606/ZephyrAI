#pragma once

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "cg_enums.h"
#include <type_traits>

#include "dsp/basic_math_functions.h"

using namespace arm_cmsis_stream;

template <typename IN1, int inputSize1,
          typename IN2, int inputSize2,
          typename OUT2, int outputSize2>
class StereoToMono;

template <typename T, int inputSamples>
class StereoToMono<T, inputSamples, T, inputSamples, T, inputSamples> : public GenericNode21<T, inputSamples, T, inputSamples, T, inputSamples>
{

  public:
    StereoToMono(FIFOBase<T> &left, FIFOBase<T> &right, FIFOBase<T> &dst)
        : GenericNode21<T, inputSamples, T, inputSamples, T, inputSamples>(left, right, dst) {};


    int run() final
    {
        T *o = this->getWriteBuffer();
        T *l = this->getReadBuffer1();
        T *r = this->getReadBuffer2();

        if constexpr (std::is_same_v<T, float>) 
        {
           arm_scale_f32(l, 0.5f, l, inputSamples);
           arm_scale_f32(r, 0.5f, r, inputSamples);
           arm_add_f32(l, r, o, inputSamples);
        }
        else if constexpr (std::is_same_v<T, q15_t>) 
        {
           arm_shift_q15(l, -1, l, inputSamples); // 0.5 in Q15
           arm_shift_q15(r, -1, r, inputSamples); // 0.5 in Q15
           arm_add_q15(l, r, o, inputSamples);
        }
        else if constexpr (std::is_same_v<T, q31_t>) 
        {
           arm_shift_q31(l, -1, l, inputSamples); // 0.5 in Q31
           arm_shift_q31(r, -1, r, inputSamples); // 0.5 in Q31
           arm_add_q31(l, r, o, inputSamples);
        }
        else 
        {
           static_assert(false, "Unsupported type");
        }

        return (CG_SUCCESS);
    };
};