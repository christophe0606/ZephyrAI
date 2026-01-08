#pragma once

extern "C"
{
#include "arm_math_types.h"
#include "config.h"
#include "speex_preprocess.h"
}

#include "arm_stream_custom_config.hpp"
#include "GenericNodes.hpp"

using namespace arm_cmsis_stream;

template <typename IN, int inputSize,
          typename OUT, int outputSize>
class SpeexPreprocess;

// Size of FFT is hardcoded in speex_fft_init
template <>
class SpeexPreprocess<q15_t, 256,
          q15_t, 256> : public GenericNode<q15_t, 256,
                                             q15_t, 256>
{
  public:
    SpeexPreprocess(FIFOBase<q15_t> &src,
        FIFOBase<q15_t> &dst)
        : GenericNode<q15_t, 256,
                      q15_t, 256>(src, dst)
    {

        p_state = speex_preprocess_state_init(256, SAMPLE_RATE);
        if (p_state == NULL)
        {
            ERROR_PRINT("Failed to initialize SpeexPreprocess\n");
        }
        else
        {
            speex_preprocess_ctl(p_state, SPEEX_PREPROCESS_SET_ECHO_STATE, NULL);
            spx_int32_t enabled = 1;
            spx_int32_t disabled = 0;
            speex_preprocess_ctl(p_state, SPEEX_PREPROCESS_SET_DENOISE, (spx_int32_t*)&enabled);
            speex_preprocess_ctl(p_state, SPEEX_PREPROCESS_SET_AGC, (spx_int32_t*)&disabled);
            speex_preprocess_ctl(p_state, SPEEX_PREPROCESS_SET_DEREVERB, (spx_int32_t*)&disabled);

        }
    };

    ~SpeexPreprocess()
    {
        if (p_state)
        {
            speex_preprocess_state_destroy(p_state);
            p_state = NULL;
        }
    }

    int prepareForRunning() final
    {
        if (this->willOverflow() ||
            this->willUnderflow())
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {

        q15_t *in = this->getReadBuffer();
        q15_t *p_in_place_buffer = this->getWriteBuffer();

        memcpy(p_in_place_buffer, in, sizeof(q15_t) * 256);

        speex_preprocess_run(p_state, p_in_place_buffer);

        return (CG_SUCCESS);
    };

  protected:
    SpeexPreprocessState *p_state;
};