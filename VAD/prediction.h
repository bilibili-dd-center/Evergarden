#pragma once

#include <tensorflow/c/c_api.h>
#include <string>
#include <memory>

#include "datafeeder.h"
#include "mrcg.h"
#include "../exceptions.h"

namespace Evergarden::VAD::Prediction {
	const int nbDNNInputs = 3, nbDNNOutputs = 1;
    const auto StatusDeleter = [](TF_Status *status) { TF_DeleteStatus(status); };
    const auto GraphDeleter = [](TF_Graph *g) { TF_DeleteGraph(g); };
    const auto BufferDeleter = [](TF_Buffer *buffer) { TF_DeleteBuffer(buffer); };
    const auto GraphOptsDeleter = [](TF_ImportGraphDefOptions *opts) { TF_DeleteImportGraphDefOptions(opts); };
    const auto SessionOptsDeleter = [](TF_SessionOptions *opts) { TF_DeleteSessionOptions(opts); };
    const auto TensorDeleter = [](TF_Tensor *tensor) { TF_DeleteTensor(tensor); };

    class VADPrediction {
    public:
        VADPrediction();
        ~VADPrediction();
        void Predict(float *input, int nWindows, int nChannels, int batchSize, float *output);
    private:
		TF_Session *session;
        TF_Graph *graph;
        std::unique_ptr<TF_Graph, decltype(GraphDeleter)> graphGuard;

		TF_Output *inputs, *outputs;
		std::unique_ptr<TF_Output> inputsGuard, outputsGuard;
    };
}
