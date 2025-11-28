#pragma once

#include <cstdint>
#include CMSIS_device_header

#include <new>

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "custom.hpp"

#include "cmsis_os2.h"

#include "node_globals.hpp"

using namespace arm_cmsis_stream;

#define VSTREAM_STEREO_SINK_BLOCK_COUNT (3)

extern "C"
{
    extern osThreadId_t tid_stream;
#include "Driver_I2C.h"

#include "Driver_SAI.h"
#include "WM8904_driver.h"
#include "rtos_events.hpp"

    extern ARM_DRIVER_WM8904 WM8904;
    extern ARM_DRIVER_SAI Driver_SAI2;
#define AudioSinkDriver (&Driver_SAI2)
}


extern ARM_DRIVER_I2C ARM_Driver_I2C_(RTE_WM8904_CODEC_I2C_INSTANCE);
#define WM8904_I2C (&ARM_Driver_I2C_(RTE_WM8904_CODEC_I2C_INSTANCE))

template <typename OUT, int outputSize>
class VStreamAudioSink;

template <int inputSamples>
class VStreamAudioSink<sq15, inputSamples>
    : public GenericSink<sq15, inputSamples>
{
  public:
    enum selector
    {
        selVolume = 0
    };
    static std::array<uint16_t, 1> selectors;

    static void ARM_SAI_SignalEvent(uint32_t event)
    {
        if (g_audioOutputState.audioBuffer == nullptr)
            return;

        // Handle SAI events
        if (event & ARM_SAI_EVENT_SEND_COMPLETE)
        {

            void *buf = g_audioOutputState.audioBuffer + g_audioOutputState.transmitIndex * g_audioOutputState.bufferSize;
            g_audioOutputState.transmitIndex = (g_audioOutputState.transmitIndex + 1) % VSTREAM_STEREO_SINK_BLOCK_COUNT;
            if (g_audioOutputState.transmitIndex == g_audioOutputState.currentIndex)
            {
                ERROR_PRINT("Data underflow t=%d, c=%d\n", g_audioOutputState.transmitIndex, g_audioOutputState.currentIndex); // Handle SAI Tx underflow

                // Underflow, no new data available
                if (tid_stream != NULL)
                    osThreadFlagsSet(tid_stream, AUDIO_SINK_UNDERFLOW_EVENT);
                return;
            }

            if (AudioSinkDriver->Send(buf, 2*inputSamples ) != ARM_DRIVER_OK)
            {
                ERROR_PRINT("Failed to send audio data 2\n");
                if (tid_stream != NULL)
                    osThreadFlagsSet(tid_stream, AUDIO_SINK_UNDERFLOW_EVENT);
            }

            if (tid_stream != NULL)
                osThreadFlagsSet(tid_stream, AUDIO_SINK_FRAME_EVENT);
        }
        if (event & ARM_SAI_EVENT_TX_UNDERFLOW)
        {
            ERROR_PRINT("SAI TX underflow"); // Handle SAI Tx underflow
            osThreadFlagsSet(tid_stream, AUDIO_SINK_UNDERFLOW_EVENT);
        }
    }

    VStreamAudioSink(FIFOBase<sq15> &dst, int volume = 5, int master = 1)
        : GenericSink<sq15, inputSamples>(dst), master_(master)
    {

        g_audioOutputState.currentIndex = VSTREAM_STEREO_SINK_BLOCK_COUNT - 1;
        g_audioOutputState.transmitIndex = 0;
        g_audioOutputState.bufferSize = sizeof(sq15) * inputSamples;
        stereoBuffer = new (std::align_val_t(64)) sq15[VSTREAM_STEREO_SINK_BLOCK_COUNT * inputSamples];
        g_audioOutputState.audioBuffer = (uint8_t *)stereoBuffer;

        if (AudioSinkDriver->Initialize(ARM_SAI_SignalEvent) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to initialize SAI driver\n");
            initErrorOccured = true;
            delete[] (stereoBuffer);
            g_audioOutputState.audioBuffer = nullptr;
            goto endInit;
        }

        if (AudioSinkDriver->PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to power on SAI driver\n");
            initErrorOccured = true;
            delete[] (stereoBuffer);
            g_audioOutputState.audioBuffer = nullptr;
            AudioSinkDriver->Uninitialize();
            goto endInit;
        }

        // SAI for Alif implementation but it looks likes it is not
        // really implementing the SAI API (it looks different from SAI documentation)
        if (AudioSinkDriver->Control(ARM_SAI_CONFIGURE_TX |
                                         ARM_SAI_MODE_MASTER |
                                         ARM_SAI_ASYNCHRONOUS |
                                         ARM_SAI_PROTOCOL_I2S |
                                        ARM_SAI_DATA_SIZE(16),
                                    32, 48000) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to configure SAI driver\n");
            initErrorOccured = true;
            delete[] (stereoBuffer);
            g_audioOutputState.audioBuffer = nullptr;
            AudioSinkDriver->PowerControl(ARM_POWER_OFF);
            AudioSinkDriver->Uninitialize();
            goto endInit;
        }

        if (WM8904.Initialize() != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to initialize WM8904 codec\n");
            initErrorOccured = true;
            delete[] (stereoBuffer);
            g_audioOutputState.audioBuffer = nullptr;
            AudioSinkDriver->PowerControl(ARM_POWER_OFF);
            AudioSinkDriver->Uninitialize();
            goto endInit;
        }

        if (WM8904.PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to power on WM8904 codec\n");
            initErrorOccured = true;
            delete[] (stereoBuffer);
            g_audioOutputState.audioBuffer = nullptr;
            WM8904.Uninitialize();
            AudioSinkDriver->PowerControl(ARM_POWER_OFF);
            AudioSinkDriver->Uninitialize();
            goto endInit;
        }

endInit:        setVolume(volume);
    };

    ~VStreamAudioSink()
    {
        if (initErrorOccured)
            return;
        if (AudioSinkDriver->Control(ARM_SAI_CONTROL_TX, 0, 0) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to abort SAI send\n");
        }
        if (AudioSinkDriver->PowerControl(ARM_POWER_OFF) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to power off SAI driver\n");
        }
        if (AudioSinkDriver->Uninitialize() != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to uninitialize SAI driver\n");
        }
        if (WM8904.Uninitialize() != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to uninitialize WM8904 codec\n");
        }
        delete[] (stereoBuffer);
    };

    void setVolume(int v)
    {
        if (initErrorOccured)
            return;
        if (v < 0)
            v = 0;
        if (v > 100)
            v = 100;
        if (WM8904.SetVolume(v) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to set volume on WM8904 codec\n");
        }
    }

    void processEvent(int dstPort, Event &&evt) final override
    {
        if (evt.event_id == selectors[selVolume])
        {
            if (evt.wellFormed<int>())
            {
                evt.apply<int>(&VStreamAudioSink::setVolume, *this);
            }
        }
    }

    int prepareForRunning() final
    {
        if (this->willUnderflow())
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {
        if (initErrorOccured == true)
            return (CG_INIT_FAILURE);

        if (!started)
        {
            started = true;
            int32_t status = status = AudioSinkDriver->Control(ARM_SAI_CONTROL_TX, 1, 0);
            if (status)
            {
                // Would be better to have dedicated error code
                // But it must be different from CG_INIT_FAILURE
                ERROR_PRINT("Failed to start SAI TX\n");
                return CG_OS_ERROR;
            }

            if (AudioSinkDriver->Send(g_audioOutputState.audioBuffer, 2*inputSamples) != ARM_DRIVER_OK)
            {
                ERROR_PRINT("Failed to send audio data 1\n");
                return (CG_BUFFER_UNDERFLOW);
            }
            //uint32_t ticks = osKernelGetTickCount();
            //uint32_t tickFreq = osKernelGetTickFreq();
            //printf("sink start time %f ms\n",1.0f * ticks / tickFreq * 1000.0f);

        }
       
        sq15 *buf = getCurrentBlock();
        sq15 *input = this->getReadBuffer();
        if (buf)
            memcpy(buf, input, inputSamples * sizeof(sq15));
        else
            return (CG_BUFFER_OVERFLOW);

        if (master_)
        {
            uint32_t flags = osThreadFlagsWait(AUDIO_SINK_FRAME_EVENT | AUDIO_SINK_UNDERFLOW_EVENT, osFlagsWaitAny, osWaitForever);
            if (flags & AUDIO_SINK_UNDERFLOW_EVENT)
            {
                return (CG_BUFFER_UNDERFLOW);
            }
        }
        //uint32_t ticks = osKernelGetTickCount();
        //uint32_t tickFreq = osKernelGetTickFreq();
        //printf("sink time %f ms\n",1.0f * ticks / tickFreq * 1000.0f);


        return (CG_SUCCESS);
    };

  protected:
    sq15 *getCurrentBlock()
    {
        if (g_audioOutputState.currentIndex == g_audioOutputState.transmitIndex)
        {
            // No new block available
            return nullptr;
        }
        uint32_t oldIndex = g_audioOutputState.currentIndex;
        g_audioOutputState.currentIndex = (g_audioOutputState.currentIndex + 1) % VSTREAM_STEREO_SINK_BLOCK_COUNT;

        return &stereoBuffer[oldIndex * inputSamples];
    }

    bool started{false};
    sq15 *stereoBuffer;
    int master_;
    bool initErrorOccured{false};
};