#pragma once

#include <string>
#include <cstring>
#include <vector>

#ifdef _MSC_VER
#define NOMINMAX
#endif
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/platform/env.h>
using namespace tensorflow;

#include "datafeeder.h"
#include "mrcg.h"
namespace Evergarden::Prediction {
	const std::string VADModelFilename = "vad_model.pb";

	class VADPrediction {
	public:
		VADPrediction(float *input, int nWindows, int batchSize, int nChannels = Evergarden::MRCG::defaultResultNChannels);
		~VADPrediction();

		void Predict();
	private:
		GraphDef graphDef;
		Session *session;

		DataFeeder<float> feeder;

		int nChannels;
		int batchSize;

		Tensor inputTensor;
		Tensor labelTensor;
		Tensor keepProbabilityTensor;
		std::vector<std::pair<std::string, Tensor>> inputs;
		std::vector<std::string> outputTensorNames = { "import/model_1/logits:0" };
		std::vector<std::string> targetNodeNames = { };

		float *labelBuf, *inputBuf;
	};

	class PredictionException : public std::exception
	{
	public:
		const char* what() const noexcept override
		{
			return message.c_str();
		}

		explicit PredictionException(std::string s) : message("[Evergarden::Prediction] " + std::move(s)) { }

	private:
		const std::string message;
	};
}