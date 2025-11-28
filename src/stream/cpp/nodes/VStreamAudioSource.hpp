#pragma once

#include CMSIS_device_header

#include <new>

#include "cg_enums.h"
#include "custom.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"

#include "cmsis_os2.h"
#include "cmsis_vstream.h"
#include "rtos_events.hpp"
#include "node_globals.hpp"


using namespace arm_cmsis_stream;

#define VSTREAM_STEREO_SOURCE_BLOCK_COUNT (2)

extern vStreamDriver_t Driver_vStreamAudioIn;
#define vStream_AudioIn (&Driver_vStreamAudioIn)

extern "C" {
    extern osThreadId_t tid_stream;
}

template <typename OUT, int outputSize>
class VStreamAudioSource;

template <int outputSamples>
class VStreamAudioSource<sq15, outputSamples>
    : public GenericSource<sq15, outputSamples>
{
  public:
    static void AudioSourceDrv_Event_Callback(uint32_t event)
    {
        (void)event;
        if (event & VSTREAM_EVENT_OVERFLOW)
        {
            if (tid_stream != NULL)
                osThreadFlagsSet(tid_stream, AUDIO_SOURCE_OVERFLOW_EVENT);
        }

        if (tid_stream != NULL)
           osThreadFlagsSet(tid_stream, AUDIO_SOURCE_FRAME_EVENT);
    }

    VStreamAudioSource(FIFOBase<sq15> &dst,int master=1)
        : GenericSource<sq15, outputSamples>(dst), master_(master)
    {

        stereoBuffer = new (std::align_val_t(64)) sq15[VSTREAM_STEREO_SOURCE_BLOCK_COUNT * outputSamples];
        
        /* Initialize audio in stream and set the receive buffer */
        if (vStream_AudioIn->Initialize(AudioSourceDrv_Event_Callback) != VSTREAM_OK)
        {
            ERROR_PRINT("vStream_AudioIn Initialize error\n");
            initErrorOccurred = true;
            delete[] (stereoBuffer);
        }
        
        if (vStream_AudioIn->SetBuf(stereoBuffer,
                                VSTREAM_STEREO_SOURCE_BLOCK_COUNT * sizeof(sq15) * outputSamples,
                                sizeof(sq15) * outputSamples) != VSTREAM_OK)
        {
            ERROR_PRINT("vStream_AudioIn SetBuf error\n");
            initErrorOccurred = true;
            vStream_AudioIn->Uninitialize();
            delete[] (stereoBuffer);
        }

        
    };

   
    ~VStreamAudioSource()
    {
        /* Stop audio receiver */
        vStream_AudioIn->Stop();
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
        if (!started)
        {
            started = true;
            if (vStream_AudioIn->Start(VSTREAM_MODE_CONTINUOUS) != VSTREAM_OK)
            {
                 ERROR_PRINT("vStream_AudioIn Start error\n");
                 return(CG_INIT_FAILURE);
            }
            //uint32_t ticks = osKernelGetTickCount();
            //uint32_t tickFreq = osKernelGetTickFreq();
            //printf("src start time %f ms\n",1.0f * ticks / tickFreq * 1000.0f);

            osThreadFlagsWait(AUDIO_SOURCE_FRAME_EVENT|AUDIO_SOURCE_OVERFLOW_EVENT, osFlagsWaitAny, osWaitForever);
            
        }
        else if (master_)
        {
            uint32_t flags = osThreadFlagsWait(AUDIO_SOURCE_FRAME_EVENT|AUDIO_SOURCE_OVERFLOW_EVENT, osFlagsWaitAny, osWaitForever);
            if (flags & AUDIO_SOURCE_OVERFLOW_EVENT)
            {
                return (CG_BUFFER_OVERFLOW);
            }
        }

        //uint32_t ticks = osKernelGetTickCount();
        //uint32_t tickFreq = osKernelGetTickFreq();
        //printf("src time %f ms\n",1.0f * ticks / tickFreq * 1000.0f);

        
        sq15 *buf = (sq15 *)vStream_AudioIn->GetBlock();
        sq15 *out = this->getWriteBuffer();
        if (buf)
        {
            memcpy(out, buf, outputSamples * sizeof(sq15));
            vStream_AudioIn->ReleaseBlock();

        }
        else 
        {
            ERROR_PRINT("vStream_AudioIn GetBlock error\n");
            return(CG_BUFFER_UNDERFLOW);
        }

        return (CG_SUCCESS);
    };

  protected:
    bool started{false};
    sq15 *stereoBuffer;
    int master_;
    bool initErrorOccurred{false};
};