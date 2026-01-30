#pragma once
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "dsp/basic_math_functions.h"

#include <string>

extern "C" {
#include "arm_vec_math.h"
#include "dsp/fast_math_functions.h"
#include "dsp/statistics_functions.h"
#include "node_settings_datatype.h"
}

using namespace arm_cmsis_stream;

class KWSClassify: public StreamNode, public ContextSwitch
{
	static constexpr size_t nbLabels = 12;
	static constexpr size_t historySizeDefault = 4;
	static constexpr const char *labelsVec[nbLabels] = {
		"down",  "go",   "left", "no",  "off",       "on",
		"right", "stop", "up",   "yes", "_silence_", "_unknown_",
	};

      public:
	KWSClassify(EventQueue *queue, const struct classifyParams &params)
		: StreamNode(), ev0(queue), historySize_(params.historyLength)
	{
		history.resize(params.historyLength + 1);
		for (auto &v : history) {
			v.resize(nbLabels, 0.0f);
		}
	};

	int pause() final override
	{
		for (auto &v : history) {
			std::fill(v.begin(), v.end(), 0.0f);
		}
		return 0;
	}

	int resume() final override
	{

		return 0;
	}

	static void softmax(float *in, size_t blockSize)
	{
		float32_t maxVal;
		const float32_t *pIn;
		int32_t blkCnt;
		float32_t accum = 0.0f;
		float32_t tmp;

		arm_max_no_idx_f32((float32_t *)in, blockSize, &maxVal);

		blkCnt = blockSize;
		pIn = in;

		f32x4_t vSum = vdupq_n_f32(0.0f);
		blkCnt = blockSize >> 2;
		while (blkCnt > 0) {
			f32x4_t vecIn = vld1q(pIn);
			f32x4_t vecExp;

			vecExp = vexpq_f32(vsubq_n_f32(vecIn, maxVal));

			vSum = vaddq_f32(vSum, vecExp);

			/*
			 * Decrement the blockSize loop counter
			 * Advance vector source and destination pointers
			 */
			pIn += 4;
			blkCnt--;
		}

		/* sum + log */
		accum = vecAddAcrossF32Mve(vSum);

		blkCnt = blockSize & 0x3;
		while (blkCnt > 0) {
			tmp = *pIn++;
			accum += expf(tmp - maxVal);
			blkCnt--;
		}

		tmp = 1.0f / accum;
		arm_scale_f32((const float32_t *)in, tmp, in, blockSize);
	}

	virtual ~KWSClassify()
	{
	}

	void sendLabel(int c)
	{
		if (c < 0) {
			return;
		}

		uint32_t label_idx = static_cast<uint32_t>(c);
		if (label_idx < nbLabels - 2) {
			const char *a = labelsVec[label_idx];
			if (label_idx != lastRec) {
				LOG_INF("KWS Classify: %s\n", a);
			}
			lastRec = label_idx;
			ev0.sendSync(kNormalPriority, kValue,
				     (uint32_t)label_idx); // Send the event to the subscribed nodes
		}
	}

	int computeClass(const float *t)
	{
		memcpy(buf, t, nbLabels * sizeof(float));
		// softmax
		softmax(buf, nbLabels);
		// add array to history
		for (int i = historySize_ - 1; i > 0; i--) {
			history[i] = std::move(history[i - 1]);
		}
		history[0] = std::vector<float>(buf, buf + nbLabels);

		memset(buf, 0, nbLabels * sizeof(float));
		for (const auto &v : history) {
			arm_add_f32(v.data(), buf, buf, nbLabels);
		}

		// find max
		uint32_t index;
		float res;
		arm_max_f32(buf, nbLabels, &res, &index);
		return index;
	}

	void processKWS(const TensorPtr<float> &t)
	{
		int res = -1;
		bool lockError;
		t.lock_shared(lockError, [&res, this](const Tensor<float> &tensor) {
			const float *buf = tensor.buffer();
			res = computeClass(buf);
			sendLabel(res);
		});
	}

	void processConstantKWS(const TensorPtr<const float> &t)
	{
		int res = -1;
		bool lockError;
		t.lock_shared(lockError, [&res, this](const Tensor<const float> &tensor) {
			const float *buf = tensor.buffer();
			res = computeClass(buf);
			sendLabel(res);
		});
	}

	void processEvent(int dstPort, Event &&evt) final override
	{
		if (evt.event_id == kValue) {
			if (evt.wellFormed<TensorPtr<float>>()) {
				evt.apply<TensorPtr<float>>(&KWSClassify::processKWS, *this);
			}
			if (evt.wellFormed<TensorPtr<const float>>()) {
				evt.apply<TensorPtr<const float>>(&KWSClassify::processConstantKWS,
								  *this);
			}
		}
	}

	void subscribe(int outputPort, StreamNode &dst, int dstPort) final override
	{
		ev0.subscribe(dst, dstPort);
	}

      protected:
	uint32_t lastRec{11};
	float buf[nbLabels];
	std::vector<std::vector<float>> history;
	EventOutput ev0;
	size_t historySize_;
};