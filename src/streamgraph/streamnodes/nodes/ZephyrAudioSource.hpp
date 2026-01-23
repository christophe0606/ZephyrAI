/* Based upon AudioBackend.cpp from Alif kws example.
 * Highly modified for use in CMSIS Stream.
 * Modifications Copyright (c) 2025-2026 Arm Limited or its affiliates. All rights reserved.
 * Original license:
 * Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#pragma once

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>

#include <new>

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
extern "C" {
#include "node_settings_datatype.h"
}

#include "init_drv_src.hpp"

#include <atomic>

using namespace arm_cmsis_stream;

template <typename OUT, int outputSize> class ZephyrAudioSource;

template <int outputSamples>
class ZephyrAudioSource<sq15, outputSamples> : public GenericSource<sq15, outputSamples>, public ContextSwitch
{
	static_assert(CONFIG_I2S_SAMPLES == outputSamples,
		      "The audio source output size must match CONFIG_I2S_SAMPLES");

      public:
	ZephyrAudioSource(FIFOBase<sq15> &dst, const struct hardwareParams &settings)
		: GenericSource<sq15, outputSamples>(dst), settings_(settings)
	{
		
	};

	int pause() final
	{
		// Implementation of pause
		if (started_.load() == false) {
			// If it was never started, nothing to do
			return 0;
		}
		int rc = i2s_trigger(settings_.i2s_mic, I2S_DIR_RX, I2S_TRIGGER_STOP);
		if (rc < 0) {
			LOG_ERR("I2S_TRIGGER_STOP failed: %i", rc);
		}
		started_.store(false);
		return 0;
	}

	int resume() final
	{
		// Implementation of resume
		return 0;
	}

	
	int run() final
	{
		size_t size;
		if (!started_.load()) {
			LOG_DBG("Starting RX");
			
			int rc = i2s_trigger(settings_.i2s_mic, I2S_DIR_RX, I2S_TRIGGER_START);

			if (rc < 0) {
				LOG_ERR("i2s_trigger start failed: %i", rc);
				return (CG_INIT_FAILURE);
			}
			started_.store(true);
		}

		sq15 *out = this->getWriteBuffer();
		memset(out, 0, outputSamples * sizeof(sq15));
		int err=0;
		void *buffer = NULL;
		//err = i2s_buf_read(i2s_mic, out, &size);
		err = i2s_read(settings_.i2s_mic, &buffer, &size);
		

		if (err != 0) {
			LOG_ERR("i2s_buf_read failed: %d", err);
			k_mem_slab_free(settings_.mem_slab, buffer);
            stop_audio();
			return (CG_BUFFER_UNDERFLOW);
		}

		memcpy(out, buffer, size);
		k_mem_slab_free(settings_.mem_slab, buffer);
		return (CG_SUCCESS);
	};

      protected:
	void stop_audio()
	{
		int rc = i2s_trigger(settings_.i2s_mic, I2S_DIR_RX, I2S_TRIGGER_DROP);
		if (rc < 0) {
			LOG_ERR("I2S_TRIGGER_DROP failed: %i", rc);
		}
		started_.store(false);
	}
	std::atomic<bool> started_ = false;
	const struct hardwareParams &settings_;
};