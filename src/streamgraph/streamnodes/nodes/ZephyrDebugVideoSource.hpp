#pragma once

#include <zephyr/kernel.h>

#include <new>

#include "cg_enums.h"
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"

#include <atomic>

using namespace arm_cmsis_stream;

#define DBG_VIDEO_WIDTH      100
#define DBG_VIDEO_HEIGHT     50
#define DBG_VIDEO_FRAME_SIZE (DBG_VIDEO_WIDTH * DBG_VIDEO_HEIGHT * 2) // RGB565

K_MEM_SLAB_DEFINE_STATIC(video_slab, DBG_VIDEO_FRAME_SIZE, 2, 4);

struct DebugVideoCtx {
  struct k_work_delayable dwork;
  void *owner;  
};

// RGB565
class ZephyrDebugVideoSource: public StreamNode, public ContextSwitch
{
      public:
	ZephyrDebugVideoSource(EventQueue *queue) : StreamNode(), queue_(queue), ev(queue)
	{
		dt = 1.0f / 10.0f;
		val = 0.0f;

		ctx_.owner = this;
        k_work_init_delayable(&ctx_.dwork, &ZephyrDebugVideoSource::refresh);
	}

	static void release_video_frame(void *frame)
	{
		k_mem_slab_free(&video_slab, frame);
	}

	static void refresh(struct k_work *work)
	{
		struct k_work_delayable *dwork = k_work_delayable_from_work(work);

		DebugVideoCtx *ctx = CONTAINER_OF(dwork, DebugVideoCtx, dwork);
        ZephyrDebugVideoSource *self = static_cast<ZephyrDebugVideoSource *>(ctx->owner);

        if (self->must_refresh_.load())
		{
			//LOG_DBG("Refresh\n");
			self->queue_->push(LocalDestination{self, 0}, Event(kDo, kNormalPriority));

		    k_work_schedule(dwork, K_MSEC(100));
	    }
	}

	
	int pause() final
	{		
		must_refresh_.store(false);
		k_work_cancel_delayable(&ctx_.dwork);
		return 0;
	}

	int resume() final
	{
		val = 0.0f;
		must_refresh_.store(true);
		k_work_schedule(&ctx_.dwork, K_MSEC(100));
		return 0;
	}


	void processEvent(int dstPort, Event &&evt) final override
	{
		if ((evt.event_id == kDo) && (must_refresh_.load())) {
			uint16_t *frameBuffer_ = nullptr;
			k_mem_slab_alloc(&video_slab, (void **)&frameBuffer_, K_NO_WAIT);
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
				for (int y = 0; y < DBG_VIDEO_HEIGHT; ++y) {
					for (int x = 0; x < DBG_VIDEO_WIDTH; ++x) {
						((uint16_t *)
							 frameBuffer_)[y * DBG_VIDEO_WIDTH + x] =
							color;
					}
				}

				UniquePtr<const uint16_t> tensorData((const uint16_t *)frameBuffer_,
								     release_video_frame);
				TensorPtr<const uint16_t> t =
					TensorPtr<const uint16_t>::create_with(
						(uint8_t)2,
						cg_tensor_dims_t{DBG_VIDEO_HEIGHT, DBG_VIDEO_WIDTH},
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
	DebugVideoCtx ctx_;
	EventQueue *queue_;
	EventOutput ev;
	float dt;
	float val;
	std::atomic<bool> must_refresh_{false};
};
