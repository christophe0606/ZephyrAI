/*
 * Copyright (C) 2024 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 */

/*

Alif video example modified for inclusion in CMSIS Stream

*/
#pragma once

#include <zephyr/kernel.h>

#include <new>

#include "cg_enums.h"
#include "arm_stream_custom_config.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"

#include <zephyr/device.h>

#include <zephyr/drivers/video.h>
#include <zephyr/logging/log.h>

using namespace arm_cmsis_stream;

#define N_FRAMES   1
#define N_VID_BUFF MIN(CONFIG_VIDEO_BUFFER_POOL_NUM_MAX, N_FRAMES)

#define FORMAT_TO_CAPTURE VIDEO_PIX_FMT_RGB565
#define MAX_WIDTH         480
#define MAX_HEIGHT        272

// RGB565
class ZephyrVideoSource : public StreamNode
{
      public:
	ZephyrVideoSource() : StreamNode()
	{
		dt = 1.0f / 10.0f;
		val = 0.0f;
	}

	cg_status init() final override
	{
		int i = 0;
		int ret;
		size_t bsize;

		video = DEVICE_DT_GET_ONE(alif_cam);
		if (!device_is_ready(video)) {
			LOG_ERR("%s: device not ready.", video->name);
			return (CG_INIT_FAILURE);
		}

		LOG_INF("- Device name: %s\n", video->name);

		/* Get capabilities */
		if (video_get_caps(video, VIDEO_EP_OUT, &caps)) {
			LOG_ERR("Unable to retrieve video capabilities");
			return (CG_INIT_FAILURE);
		}

		LOG_INF("- Capabilities:\n");
		while (caps.format_caps[i].pixelformat) {
			const struct video_format_cap *fcap = &caps.format_caps[i];

			LOG_INF("  %c%c%c%c width (min, max, step)[%u; %u; %u] "
			"height (min, max, step)[%u; %u; %u]\n",
		       (char)fcap->pixelformat,
		       (char)(fcap->pixelformat >> 8),
		       (char)(fcap->pixelformat >> 16),
		       (char)(fcap->pixelformat >> 24),
		       fcap->width_min, fcap->width_max, fcap->width_step,
		       fcap->height_min, fcap->height_max, fcap->height_step);

			if (fcap->pixelformat == FORMAT_TO_CAPTURE) {
				if (MAX_WIDTH == fcap->width_max  &&
				    MAX_HEIGHT == fcap->height_max) {
					fmt.pixelformat = FORMAT_TO_CAPTURE;
					fmt.width = MAX_WIDTH;
					fmt.height = MAX_HEIGHT;
				}
			}
			i++;
		}

		if (fmt.pixelformat == 0) {
			LOG_ERR("Desired Pixel format is not supported.");
			return (CG_INIT_FAILURE);
		}

		/* fourcc to string */
		LOG_INF("  %c%c%c%c width [%u] "
			"height [%u]\n",
			(char)fmt.pixelformat, (char)(fmt.pixelformat >> 8),
			(char)(fmt.pixelformat >> 16), (char)(fmt.pixelformat >> 24),
			fmt.width, fmt.height);

		switch (fmt.pixelformat) {
		case VIDEO_PIX_FMT_RGB565:
			fmt.pitch = fmt.width << 1;
			break;
		case VIDEO_PIX_FMT_Y10P:
			fmt.pitch = fmt.width;
			break;
		case VIDEO_PIX_FMT_BGGR8:
		case VIDEO_PIX_FMT_GBRG8:
		case VIDEO_PIX_FMT_GRBG8:
		case VIDEO_PIX_FMT_RGGB8:
		case VIDEO_PIX_FMT_GREY:
		default:
			fmt.pitch = fmt.width;
			break;
		}

		ret = video_set_format(video, VIDEO_EP_OUT, &fmt);
		if (ret) {
			LOG_ERR("Failed to set video format. ret - %d", ret);
			return (CG_INIT_FAILURE);
		}

		LOG_INF("- format: %c%c%c%c %ux%u\n", (char)fmt.pixelformat,
			(char)(fmt.pixelformat >> 8), (char)(fmt.pixelformat >> 16),
			(char)(fmt.pixelformat >> 24), fmt.width, fmt.height);

		/* Size to allocate for each buffer */
		bsize = fmt.pitch * fmt.height;

		LOG_INF("Width - %d, Pitch - %d, Height - %d, Buff size - %d\n", fmt.width,
			fmt.pitch, fmt.height, bsize);

		/* Alloc video buffers and enqueue for capture */
		for (i = 0; i < ARRAY_SIZE(buffers); i++) {
			buffers[i] = video_buffer_alloc(bsize);
			if (buffers[i] == NULL) {
				LOG_ERR("Unable to alloc video buffer");
				return (CG_INIT_FAILURE);
			}

			/* Allocated Buffer Information */
			LOG_INF("- addr - 0x%x, size - %d, bytesused - %d\n",
				(uint32_t)buffers[i]->buffer, bsize, buffers[i]->bytesused);

			memset(buffers[i]->buffer, 0, sizeof(char) * bsize);
			video_enqueue(video, VIDEO_EP_OUT, buffers[i]);

			LOG_INF("capture buffer[%d]: dump binary memory "
				"\"/home/$USER/capture_%d.bin\" 0x%08x 0x%08x -r\n\n",
				i, i, (uint32_t)buffers[i]->buffer,
				(uint32_t)buffers[i]->buffer + bsize - 1);
		}

		/*
		 * TODO: Need to fix this delay.
		 * As per our observation, if we are not giving this much delay
		 * then mt9m114 camera sensor is not setup properly and images its
		 * sending out are not clear.
		 */
		k_msleep(7000);

		return (CG_SUCCESS);
	}

	static void release_video_frame(void *frame)
	{
	}

	void processEvent(int dstPort, Event &&evt) final override
	{
		if (evt.event_id == kDo) {

			uint16_t *frameBuffer_ = nullptr;
			// k_mem_slab_alloc(&video_slab, (void **)&frameBuffer_, K_NO_WAIT);
			if (frameBuffer_ != nullptr) {
				val += dt;
				if (val >= 1.0f) {
					val -= 1.0f;
				}

				uint16_t color = static_cast<uint16_t>((1.0f - val) * 0x1F);
				if (color > 0x1F) {
					color = 0x1F;
				}
				color = color << 11; // Red channel

				// Fill the frame with a test pattern
				for (uint32_t y = 0; y < fmt.height; ++y) {
					for (uint32_t x = 0; x < fmt.width; ++x) {
						((uint16_t *)frameBuffer_)[y * fmt.width + x] =
							color;
					}
				}

				UniquePtr<const uint16_t> tensorData((const uint16_t *)frameBuffer_,
								     release_video_frame);
				TensorPtr<const uint16_t> t =
					TensorPtr<const uint16_t>::create_with(
						(uint8_t)2, cg_tensor_dims_t{fmt.height, fmt.width},
						std::move(tensorData));

				ev.sendSync(kHighPriority, kValue, std::move(t));
			}
		}
	};

	void subscribe(int outputPort, StreamNode &dst, int dstPort) final override
	{
		if (outputPort == 0) {
			ev.subscribe(dst, dstPort);
		}
	}

      protected:
	EventOutput ev;
	float dt;
	float val;
	struct video_buffer *buffers[N_VID_BUFF], *vbuf;
	struct video_format fmt = {0};
	struct video_caps caps;
	const struct device *video;
};
