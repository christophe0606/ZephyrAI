#pragma once


#include "arm_stream_custom_config.hpp"
#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include <cstring>
#include <atomic>

using namespace arm_cmsis_stream;

template <typename IN, int inputSamples>
class SendToNetwork : public GenericSink<IN, inputSamples>
{
  public:
    SendToNetwork(FIFOBase<IN> &src)
        : GenericSink<IN, inputSamples>(src) {
          };

    int prepareForRunning() final
    {
        if (this->willUnderflow())
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() override final
    {
        IN *in = this->getReadBuffer();
        if (ready.load())
        {
            LOG_DBG("SendToNetwork: send buffer\n");

            UniquePtr<IN> tensorData(inputSamples);
            memcpy(tensorData.get(), in, inputSamples * sizeof(IN));

            TensorPtr<IN> t = TensorPtr<IN>::create_with((uint8_t)1,
                                                         cg_tensor_dims_t{inputSamples},
                                                         std::move(tensorData));
            bool status = ev0.sendAsync(kHighPriority, kValue, std::move(t)); // Send the event to the subscribed nodes

            if (!status)
            {
                LOG_ERR("SendToNetwork: Failed to send event to network\n");
            }
            else 
            {
                ready.store(false);
            }
        }

        return (CG_SUCCESS);
    };

    void processEvent(int dstPort, Event &&evt) final override
    {
        LOG_DBG("SendToNetwork: Event %d received on port %d\n", evt.event_id, dstPort);
        if (dstPort == 0)
        {
            if (evt.event_id == kDo)
            {
                ready.store(true);
            }
        }
    }

    void subscribe(int outputPort, StreamNode &dst, int dstPort) final override
    {
        ev0.subscribe(dst, dstPort);
    }

  protected:
    std::atomic<bool> ready{false};
    EventOutput ev0;
};