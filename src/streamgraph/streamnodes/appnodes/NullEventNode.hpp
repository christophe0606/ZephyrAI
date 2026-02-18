#pragma once
#include <atomic>
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"

using namespace arm_cmsis_stream;

class NullEventNode : public StreamNode

{

      public:
	NullEventNode()
	{
	}

	
	virtual ~NullEventNode() {};
	

    void processEvent(int dstPort, Event &&evt) final override
    {
        //LOG_INF("Debug Display: event %d\n", evt.event_id);
        if (evt.event_id == kValue)
        {
            LOG_INF("NullEventNode: Received kValue event, doing nothing.\n");
        }

        
    }

};
