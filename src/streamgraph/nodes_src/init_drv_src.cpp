#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(streamapps,CONFIG_STREAMAPPS_LOG_LEVEL);

#include "init_drv_src.hpp"

#if defined(CONFIG_DISPLAY)
extern "C" {
#include "dbuf_display/display.h"
}

#define DISPLAY_IMAGE_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t))

#endif

BUILD_ASSERT(CONFIG_I2S_SAMPLES % 4 == 0, "CONFIG_I2S_SAMPLES must be a multiple of 4");

#if defined(CONFIG_I2S)

#define I2S_DEVICE      DT_ALIAS(i2s_mic)
#define I2S_SAMPLE_SIZE sizeof(int16_t)
#define I2S_WORD_SIZE   (I2S_SAMPLE_SIZE * 8)
#define I2S_CHANNELS    2
#define I2S_SAMPLES     CONFIG_I2S_SAMPLES
#define I2S_NUM_BUFFERS CONFIG_I2S_NUM_BUFFERS
#define I2S_BUFFER_SIZE (I2S_CHANNELS * I2S_SAMPLES * I2S_SAMPLE_SIZE)

K_MEM_SLAB_DEFINE_STATIC(mem_slab, I2S_BUFFER_SIZE, I2S_NUM_BUFFERS, 4);

static const struct device *i2s_mic = DEVICE_DT_GET(I2S_DEVICE);

const struct device *init_audio_source(k_mem_slab **mem_slab_out)
{
	*mem_slab_out = &mem_slab;
	if (!device_is_ready(i2s_mic)) {
		LOG_ERR("i2s_mic is not ready");
		return nullptr;
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
		return nullptr;
	}

	return (i2s_mic);
}

#endif

#if defined(CONFIG_DISPLAY)
int init_display()
{
	void *buf = display_active_buffer();
	memset(buf, 0, DISPLAY_IMAGE_SIZE);

	buf = display_inactive_buffer();
	memset(buf, 0, DISPLAY_IMAGE_SIZE);

	int err = display_init();

	if (err==0)
	{
		display_next_frame();
	}

	return err;
	
}

void clear_display()
{
	void *buf = display_active_buffer();
	memset(buf, 0, DISPLAY_IMAGE_SIZE);

	buf = display_inactive_buffer();
	memset(buf, 0, DISPLAY_IMAGE_SIZE);

	display_next_frame();
}

#endif