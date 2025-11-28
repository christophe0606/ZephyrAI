#pragma once



#include "cg_enums.h"
#include "custom.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"


using namespace arm_cmsis_stream;


template <typename OUT, int outputSamples>
class NullSink: public GenericSink<OUT, outputSamples>
{
  public:
   
    NullSink(FIFOBase<OUT> &dst)
        : GenericSink<OUT, outputSamples>(dst)
    {

       
    };

    ~NullSink()
    {
        
    };

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
        OUT *input = this->getReadBuffer();
        (void)input; // Suppress unused variable warning
        
        return (CG_SUCCESS);
    };

  
};