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
#include "arm_stream_custom_config.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"


using namespace arm_cmsis_stream;

BUILD_ASSERT(CONFIG_I2S_SAMPLES % 4 == 0, "CONFIG_I2S_SAMPLES must be a multiple of 4");

#define I2S_DEVICE      DT_ALIAS(i2s_mic)
#define I2S_SAMPLE_SIZE sizeof(int16_t)
#define I2S_WORD_SIZE   (I2S_SAMPLE_SIZE * 8)
#define I2S_CHANNELS    2
#define I2S_SAMPLES     CONFIG_I2S_SAMPLES
#define I2S_NUM_BUFFERS CONFIG_I2S_NUM_BUFFERS
#define I2S_BUFFER_SIZE (I2S_CHANNELS * I2S_SAMPLES * I2S_SAMPLE_SIZE)

K_MEM_SLAB_DEFINE_STATIC(mem_slab, I2S_BUFFER_SIZE, I2S_NUM_BUFFERS, 4);

static const struct device *i2s_mic = DEVICE_DT_GET(I2S_DEVICE);

template <typename OUT, int outputSize> class ZephyrAudioSource;

template <int outputSamples>
class ZephyrAudioSource<sq15, outputSamples> : public GenericSource<sq15, outputSamples>
{
	static_assert(CONFIG_I2S_SAMPLES == outputSamples,
		      "The audio source output size must match CONFIG_I2S_SAMPLES");

      public:
	ZephyrAudioSource(FIFOBase<sq15> &dst, int master = 1)
		: GenericSource<sq15, outputSamples>(dst), master_(master)
	{
		
	};

	
	cg_status init() final
	{
		if (!device_is_ready(i2s_mic)) {
			LOG_ERR("i2s_mic is not ready");
			return (CG_INIT_FAILURE);
		}

		const struct i2s_config config = {
			.word_size = I2S_WORD_SIZE,
			.channels = I2S_CHANNELS,
			.format = I2S_FMT_DATA_FORMAT_I2S,
			.options = I2S_OPT_FRAME_CLK_MASTER | I2S_OPT_BIT_CLK_MASTER,
			.frame_clk_freq = static_cast<uint32_t>(CONFIG_SAMPLE_RATE),
			.mem_slab = &mem_slab,
			.block_size = I2S_BUFFER_SIZE,
			.timeout = SYS_FOREVER_MS,
		};

		int rc = i2s_configure(i2s_mic, I2S_DIR_RX, &config);

		if (rc < 0) {
			LOG_ERR("i2s_configure failed: %i", rc);
			return (CG_INIT_FAILURE);
		}

		

		return (CG_SUCCESS);
	}

	int prepareForRunning() final
	{
		if (this->willOverflow()) {
			return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
		}

		return (0);
	};

	int run() final
	{
		size_t size;
		if (!started_) {
			LOG_DBG("Starting RX");
			int rc = i2s_trigger(i2s_mic, I2S_DIR_RX, I2S_TRIGGER_START);

			if (rc < 0) {
				LOG_ERR("i2s_trigger start failed: %i", rc);
				return (CG_INIT_FAILURE);
			}
			started_ = true;
		}

		sq15 *out = this->getWriteBuffer();
		LOG_DBG("Setting outputbuffer to zero");
		memset(out, 0, outputSamples * sizeof(sq15));
		LOG_DBG("Trying to read I2S data");
		int err=0;
		void *buffer = NULL;
		//err = i2s_buf_read(i2s_mic, out, &size);
		err = i2s_read(i2s_mic, &buffer, &size);
		

		if (err != 0) {
			LOG_ERR("i2s_buf_read failed: %d", err);
			k_mem_slab_free(&mem_slab, buffer);
            stop_audio();
			return (CG_BUFFER_UNDERFLOW);
		}

		memcpy(out, buffer, size);
		k_mem_slab_free(&mem_slab, buffer);
		return (CG_SUCCESS);
	};

      protected:
	void stop_audio()
	{
		int rc = i2s_trigger(i2s_mic, I2S_DIR_RX, I2S_TRIGGER_DROP);
		if (rc < 0) {
			LOG_ERR("I2S_TRIGGER_DROP failed: %i", rc);
		}
	}
	int master_;
	bool started_ = false;
};