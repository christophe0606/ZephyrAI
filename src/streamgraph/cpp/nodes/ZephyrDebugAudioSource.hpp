#pragma once


#include <new>

#include "cg_enums.h"
#include "arm_stream_custom_config.hpp"
#include "EventQueue.hpp"
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
    ZephyrDebugAudioSource(FIFOBase<sq15> &dst,EventQueue *queue,int master=1)
        : GenericSource<sq15, outputSamples>(dst), nb_frame_(0),master_(master),ev(queue)
    {
        // Initialization code for debug audio source
    };


    int run() final
    {
        sq15 *out = this->getWriteBuffer();
        nb_frame_++;
        ev.sendSync(cg_event_priority::kNormalPriority, kValue,nb_frame_);

        for (int i = 0; i < outputSamples; i++)
        {
            out[i].left = static_cast<q15_t>(i % 32768); // Example debug data
            out[i].right = static_cast<q15_t>(i % 32768); // Example debug data
        }

        k_sleep(K_MSEC(100)); // Simulate some processing delay
        return(CG_SUCCESS);
    }

    void subscribe(int outputPort, StreamNode &dst, int dstPort) final override
    {
        if (outputPort == 0)
           ev.subscribe(dst, dstPort);
    }

protected:
    uint32_t nb_frame_;
    int master_;
    EventOutput ev;

};