#pragma once
#include "nodes/TFLite.hpp"
extern "C"
{
#include "node_settings_datatype.h"
}

class KWS : public TFLite
{
  public:
    // Array used to map local selector IDs to global selector ID
    // Global IDs are graph dependent and may change when the node is used in different graphs.
    // Here there is only one ID for the "ack" event defined in the Python
    enum selector {selAck=0};
    static std::array<uint16_t,1> selectors;

    int globalID(int localID) override final
    {
        return selectors[localID];
    }


    KWS(EventQueue *queue,const struct tfliteNodeParams &params)
        : TFLite(queue,params.modelAddr, params.modelSize,1),mNbOutputs(1) {
          };

    virtual ~KWS()
    {
    }

  protected:
    const tflite::MicroOpResolver &GetOpResolver() final override
    {
        return m_opResolver;
    }

    bool enlistOperations() final override
    {
        this->m_opResolver.AddReshape();
        this->m_opResolver.AddAveragePool2D();
        this->m_opResolver.AddConv2D();
        this->m_opResolver.AddDepthwiseConv2D();
        this->m_opResolver.AddFullyConnected();
        this->m_opResolver.AddRelu();

        if (kTfLiteOk == this->m_opResolver.AddEthosU())
        {
            LOG_DBG("Added %s support to op resolver\n",
                        tflite::GetString_ETHOSU());
        }
        else
        {
            LOG_ERR("Failed to add Arm NPU support to op resolver.");
            return false;
        }
        return true;
    }

    size_t GetNumInputs() const final override
    {
        return 1;
    }

    size_t GetNumOutputs() const final override
    {
        return mNbOutputs;
    }

    /* Maximum number of individual operations that can be enlisted. */
    static constexpr int ms_maxOpCnt = 7;
    /* A mutable op resolver instance. */
    tflite::MicroMutableOpResolver<ms_maxOpCnt> m_opResolver;
protected:
   const uint32_t mNbOutputs;
};