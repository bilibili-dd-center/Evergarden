#include "prediction_n.h"

#include <fmt/format.h>
using namespace fmt;

namespace Evergarden::Prediction {
	VADPrediction::VADPrediction(float* input, int nWindows, int batchSize, int nChannels) : 
		session(nullptr), 
		feeder(input, nWindows, nChannels, batchSize, dw, du),
		nChannels(nChannels), 
		batchSize(batchSize), 
		inputTensor(DT_FLOAT, TensorShape({ batchSize, nChannels })), 
		labelTensor(DT_FLOAT, TensorShape({ batchSize, 7 })),
		keepProbabilityTensor(DT_FLOAT, TensorShape()),
		//                                                                                                  !!!!!!!!!!this is not a typo!!!!!
		inputs({ {"import/model_1/inputs:0", inputTensor}, {"import/model_1/labels:0", labelTensor}, {"import/model_1/keep_probabilty:0", keepProbabilityTensor} }),
		labelBuf(labelTensor.flat<float>().data()),
		inputBuf(inputTensor.flat<float>().data())
	{
		auto status = ReadBinaryProto(Env::Default(), "vad_model.pb", &graphDef);
		if (!status.ok())
			throw PredictionException(
				format("Failed to load VAD prediction model file {0}: {1}! Please check if the installation is corrupted.", 
					VADModelFilename, status.error_message()));

		status = NewSession(SessionOptions(), &session);
		if (!status.ok())
			throw PredictionException(format("Failed to create Tensorflow session! {0}", status.error_message()));

		status = session->Create(graphDef);
		if (!status.ok())
			throw PredictionException(format("Failed to initialize Tensorflow session! {0}", status.error_message()));

		std::memset(labelBuf, 0, batchSize * 7 * sizeof(float));
		keepProbabilityTensor.scalar<float>()() = 1.0;
	}

	VADPrediction::~VADPrediction()
	{
		session->Close();
	}

	void VADPrediction::Predict()
	{
		while (feeder.HasNext())
		{
			feeder.FeedNext();

			std::vector<Tensor> outputs;
			if (feeder.currentBatchSize != batchSize)
			{
				Tensor inputTensor(DT_FLOAT, { feeder.currentBatchSize, nChannels });
				Tensor labelTensor(DT_FLOAT, { feeder.currentBatchSize, 7 });
				std::memset(labelTensor.flat<float>().data(), 0, batchSize * 7 * sizeof(float));
				std::memcpy(inputTensor.flat<float>().data(), feeder.currentBatchWithoutPadded, feeder.currentBatchSize * nChannels * sizeof(float));
				//                                                                                                                                                !!!!!!!!!!this is NOT a typo!!!!!
				std::vector<std::pair<std::string, Tensor>> inputs = { {"import/model_1/inputs:0", inputTensor}, {"import/model_1/labels:0", labelTensor}, {"import/model_1/keep_probabilty:0", keepProbabilityTensor} };

				auto status = session->Run(inputs, outputTensorNames, targetNodeNames, &outputs);
				if (!status.ok())
					throw PredictionException(format("Failed to perform prediction! {0}\n Position: {1}, BatchSize: {2}", status.error_message(), feeder.pos, feeder.currentBatchSize));
			} else
			{
				std::memcpy(inputBuf, feeder.currentBatchWithoutPadded, batchSize * nChannels * sizeof(float));

				auto status = session->Run(inputs, outputTensorNames, targetNodeNames, &outputs);
				if (!status.ok())
					throw PredictionException(format("Failed to perform prediction! {0}\n Position: {1}, BatchSize: {2}", status.error_message(), feeder.pos, feeder.currentBatchSize));
			}

			auto output = outputs[0].flat<float>().data();
		}
	}
}
