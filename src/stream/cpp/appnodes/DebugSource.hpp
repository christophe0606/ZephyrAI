#pragma once

#include CMSIS_device_header

#include <new>

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "custom.hpp"

#include "dsp/support_functions.h"

#include "cmsis_os2.h"
#include "cmsis_vstream.h"

#include "custom.hpp"

#include "rtos_events.hpp"

using namespace arm_cmsis_stream;

#define VSTREAM_STEREO_BLOCK_COUNT (2)

extern vStreamDriver_t Driver_vStreamAudioIn;
#define vStream_AudioIn (&Driver_vStreamAudioIn)

extern "C"
{
    extern osThreadId_t tid_stream;
}

void AudioDrv_Event_Callback(uint32_t event)
{
    (void)event;

    osThreadFlagsSet(tid_stream, AUDIO_SOURCE_FRAME_EVENT);
}

template <typename OUT, int outputSamples>
class DebugSource : public GenericSource<OUT, outputSamples>
{
  public:
    DebugSource(FIFOBase<OUT> &dst,
                int frequency = 440,
                int samplingFreq = 16000,
                int master = 0)
        : GenericSource<OUT, outputSamples>(dst), master_(master)
    {

        stereoBuffer = new (std::align_val_t(64)) OUT[VSTREAM_STEREO_BLOCK_COUNT * outputSamples];
        /* Initialize audio in stream and set the receive buffer */
        if (master_)
        {
            vStream_AudioIn->Initialize(AudioDrv_Event_Callback);
            vStream_AudioIn->SetBuf(stereoBuffer,
                                    VSTREAM_STEREO_BLOCK_COUNT * sizeof(OUT) * outputSamples,
                                    sizeof(OUT) * outputSamples);

            /* Start audio receiver */
            vStream_AudioIn->Start(VSTREAM_MODE_CONTINUOUS);
        }

        deltaPhaseFrequency = 3.141592f * 2 * frequency / samplingFreq;
        // 1 second period amplitude modulation
        deltaPhaseAmp = 3.141592f * 2 * 1 / samplingFreq;
    };

    ~DebugSource()
    {
        /* Stop audio receiver */
        if (master_)
        {
            vStream_AudioIn->Stop();
        }
        delete[] (stereoBuffer);
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
        if (master_)
        {
            osThreadFlagsWait(AUDIO_SOURCE_FRAME_EVENT, osFlagsWaitAny, osWaitForever);
            // Input audio block is read but ignore.
            // The debug source generates a known signal.
            (void)vStream_AudioIn->GetBlock();
        }
        OUT *out = this->getWriteBuffer();
        if (master_)
        {
            vStream_AudioIn->ReleaseBlock();
        }

        // Now we generate debug data
        for (int i = 0; i < outputSamples; i++)
        {
            // out[i].left = (0.3f * (cosf(phaseAmp) + 1.0f) * sinf(phaseFrequency) * 16384+0.5f);
            floatSignal[i] = 1.0f * sinf(phaseFrequency);

            phaseFrequency += deltaPhaseFrequency;
            if (phaseFrequency >= 2 * 3.141592f)
                phaseFrequency -= 2 * 3.141592f;

            phaseAmp += deltaPhaseAmp;
            if (phaseAmp >= 2 * 3.141592f)
                phaseAmp -= 2 * 3.141592f;
        }

        if constexpr (std::is_same_v<OUT, q31_t>) {
            arm_float_to_q31(floatSignal, (q31_t *)out, outputSamples);
        }
        else if constexpr (std::is_same_v<OUT, q7_t>) {
            arm_float_to_q7(floatSignal, (q7_t *)out, outputSamples);
        }
        else if constexpr (std::is_same_v<OUT, float16_t>) {
            arm_float_to_float16(floatSignal, (float16_t *)out, outputSamples);
        }
        else if constexpr (std::is_same_v<OUT, sq15>) {
            arm_float_to_q15(floatSignal, cvtBuffer, outputSamples);
            for (int i = 0; i < outputSamples; i++) {
                out[i].left = cvtBuffer[i];
                out[i].right = cvtBuffer[i];
            }
        }
        else if constexpr (std::is_same_v<OUT, q15_t>) {
            arm_float_to_q15(floatSignal, (q15_t *)out, outputSamples);
        }
        else if constexpr (std::is_same_v<OUT, sf32>) {
            for (int i = 0; i < outputSamples; i++) 
            {
                out[i].left = floatSignal[i];
                out[i].right = floatSignal[i];
            }
        }
        else if constexpr (std::is_same_v<OUT, float>) {
           memcpy(out, floatSignal, outputSamples * sizeof(OUT));
        }
        else {
            static_assert(false, "Unsupported type");
        }

        return (CG_SUCCESS);
    };

  protected:
    float32_t floatSignal[outputSamples];
    OUT cvtBuffer[outputSamples];
    OUT *stereoBuffer;
    float32_t phaseFrequency = 0.0f;
    float32_t deltaPhaseFrequency = 0.0f;
    float32_t phaseAmp = 0.0f;
    float32_t deltaPhaseAmp = 0.0f;
    bool master_;
};