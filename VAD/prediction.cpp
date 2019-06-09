#include "prediction.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstring>

#include <fmt/format.h>
using fmt::format;

namespace Evergarden::VAD::Prediction {
    const std::string VADModelFilename = "vad_model.pb";

    TF_Buffer* ReadFileToBuffer(std::string filename);

    template<typename T, typename D>
    std::unique_ptr<T, D> make_unique_with_deleter(T* t, D d)
    {
        return std::unique_ptr<T, D> (t, d);
    }

    VADPrediction::VADPrediction()
    :   graph(TF_NewGraph()), 
		graphGuard(make_unique_with_deleter(graph, GraphDeleter)) {
        auto buf = ReadFileToBuffer(VADModelFilename);
        auto bufGuard = make_unique_with_deleter(buf, BufferDeleter);

        auto status = TF_NewStatus();
        auto statusGuard = make_unique_with_deleter(status, StatusDeleter);
        auto opts = TF_NewImportGraphDefOptions();
        auto optsGuard = make_unique_with_deleter(opts, GraphOptsDeleter);
        TF_GraphImportGraphDef(graph, buf, opts, status);
        if (TF_GetCode(status) != TF_OK)
            throw PredictionException(format("Unable to import VAD graph! {0}", TF_Message(status)));

		inputs = new TF_Output[nbDNNInputs];
		inputsGuard = std::unique_ptr<TF_Output>(inputs);
		outputs = new TF_Output[nbDNNOutputs];
		outputsGuard = std::unique_ptr<TF_Output>(outputs);
		inputs[0] = { TF_GraphOperationByName(graph, "model_1/inputs"), 0 };
		inputs[1] = { TF_GraphOperationByName(graph, "model_1/labels"), 0 };
		//                                                           THIS IS NOT A TYPO
		inputs[2] = { TF_GraphOperationByName(graph, "model_1/keep_probabilty"), 0 };
		outputs[0] = { TF_GraphOperationByName(graph, "model_1/logits"), 0 };

		auto sopts = TF_NewSessionOptions();
		auto soptsGuard = make_unique_with_deleter(sopts, SessionOptsDeleter);
		session = TF_NewSession(graph, sopts, status);
		if (TF_GetCode(status) != TF_OK)
			throw PredictionException(format("Unable to create Tensorflow session! {0}", TF_Message(status)));
    }

    VADPrediction::~VADPrediction() {
		auto status = TF_NewStatus();
		auto statusGuard = make_unique_with_deleter(status, StatusDeleter);
		TF_CloseSession(session, status);
		if (TF_GetCode(status) == TF_OK)
			TF_DeleteSession(session, status);
    }

    void bDNNPredict(float* logits, int64_t logitsLen, float* output, DataFeeder<float>& feeder)
    {
		int w = feeder.w;
		int nBlocks = feeder.nbDNNBlocks;
		int* neighbors = feeder.neighbors;
		int len = feeder.currentBatchSize;
		int mx = feeder.currentBatchSize + feeder.w;
		for (int i = w; i < mx; i++)
		{
			float sum = 0;
			int cnt = 0;
			for (int col = 0; col < nBlocks; col++)
			{
				int pos = i - neighbors[nBlocks - col - 1];
				if (pos < 0 || pos >= len) continue;
				sum += logits[pos * nBlocks + col];
				cnt++;
			}
			output[i - w] = sum / cnt;
		}
    }

	void NopDeallocator(void* buf, size_t len, void* args) { }
    void VADPrediction::Predict(float *input, int nWindows, int nChannels, int batchSize, float *output) {
        DataFeeder<float> feeder(input, nWindows, nChannels, batchSize);

        auto opts = TF_NewSessionOptions();
        auto optsGuard = make_unique_with_deleter(opts, SessionOptsDeleter);
        auto status = TF_NewStatus();
        auto statusGuard = make_unique_with_deleter(status, StatusDeleter);
		auto emptyBuf = new float[feeder.rowLength * 7];
		std::memset(emptyBuf, 0, sizeof emptyBuf * sizeof(float));
		std::unique_ptr<float> emptyBufGuard(emptyBuf);

		float keepProbabilityValue = 1.0f;
		auto keepProbabilityTensor = TF_NewTensor(TF_FLOAT, nullptr, 0, &keepProbabilityValue, sizeof(float), NopDeallocator, nullptr);
		std::unique_ptr<TF_Tensor, decltype(TensorDeleter)> keepProbabilityTensorGuard = make_unique_with_deleter(keepProbabilityTensor, TensorDeleter);

		int pos = 0;
        while (feeder.HasNext()) {
            feeder.FeedNext();

			int64_t inputDims[] = { feeder.currentBatchSize, feeder.rowLength };
			int64_t labelsDims[] = { feeder.currentBatchSize, 7 };
			auto inputTensor = TF_NewTensor(TF_FLOAT, inputDims, 2, feeder.currentBatchWithoutPadded, feeder.currentBatchSize * feeder.rowLength * sizeof(float), NopDeallocator, nullptr);
			auto labelsTensor = TF_NewTensor(TF_FLOAT, labelsDims, 2, emptyBuf, feeder.currentBatchSize * 7 * sizeof(float), NopDeallocator, nullptr);
			TF_Tensor *inputTensors[] = { inputTensor, labelsTensor, keepProbabilityTensor };
			TF_Tensor *outputTensor = nullptr;

			TF_SessionRun(
				session, nullptr, 
				inputs, inputTensors, 3,
				outputs, &outputTensor, 1, 
				nullptr, 0, 
				nullptr, status);

			TF_DeleteTensor(inputTensor);
			TF_DeleteTensor(labelsTensor);

			if (TF_GetCode(status) != TF_OK)
			{
				auto msg = TF_Message(status);
				throw PredictionException(format("VAD Prediction failed! {0}", msg));
			}
				
			int64_t logitsLen = TF_TensorElementCount(outputTensor);
			auto logits = reinterpret_cast<float*>(TF_TensorData(outputTensor));

			bDNNPredict(logits, logitsLen, output + pos, feeder);

			TF_DeleteTensor(outputTensor);
			pos += feeder.currentBatchSize;
        }
    }

    void DeleteBuffer(void* data, size_t length) {
        delete[] reinterpret_cast<char*>(data);
    }

    TF_Buffer* ReadFileToBuffer(std::string filename) {
        auto path = std::filesystem::path(filename);
        if (!std::filesystem::exists(path))
            throw PredictionException(format("VAD Model {0} not found! The installation may be corrupted!", filename));

        unsigned long long length;
        try {
            length = std::filesystem::file_size(path);
        } catch (std::ios_base::failure& ex) {
            std::throw_with_nested(PredictionException(format("Failed to read VAD Model {0}! The installation may be corrupted!", filename)));
        }

        if (length > INT_MAX)
            throw PredictionException(format("File is too long! Actual size: {0}; Max Size: {1}", length, INT_MAX));

        std::unique_ptr<unsigned char> resultGuard(new unsigned char[length]);
        unsigned char *result = resultGuard.get();

        try {
            std::ifstream stream;
            std::ios_base::iostate exceptionMask = stream.exceptions() | std::ios::badbit;
            stream.exceptions(exceptionMask);

            stream.open(filename, std::ios::binary);
            if (!stream.good()) throw std::ios_base::failure("Open failed.");

            stream.read(reinterpret_cast<char*>(result), length);

            stream.close();
        } catch (std::ios_base::failure& ex) {
            std::throw_with_nested(PredictionException(format("Failed to read VAD Model {0}! The installation may be corrupted!", filename)));
        }

        TF_Buffer *buffer = TF_NewBuffer();
        buffer->length = length;
        buffer->data = result;
        buffer->data_deallocator = DeleteBuffer;

        resultGuard.release();
        return buffer;
    }
}