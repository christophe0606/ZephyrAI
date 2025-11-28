#pragma once


#include <new>

#include "cg_enums.h"
#include "custom.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"

using namespace arm_cmsis_stream;

template <typename OUT, int outputSize>
class ZephyrDebugAudioSource;

template <int outputSamples>
class ZephyrDebugAudioSource<sq15, outputSamples> : public GenericSource<sq15, outputSamples>
{
    public:
    ZephyrDebugAudioSource(FIFOBase<sq15> &dst,int master=1)
        : GenericSource<sq15, outputSamples>(dst), master_(master)
    {
        // Initialization code for debug audio source
    };

    int prepareForRunning() final
    {
        if (this->willOverflow())
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {
        sq15 *out = this->getWriteBuffer();
        printf("AUDIO SOURCE RUNNING\n");

        for (int i = 0; i < outputSamples; i++)
        {
            out[i].left = static_cast<q15_t>(i % 32768); // Example debug data
            out[i].right = static_cast<q15_t>(i % 32768); // Example debug data
        }

        k_sleep(K_MSEC(100)); // Simulate some processing delay
        return(CG_SUCCESS);
    }

protected:
    int master_;
};