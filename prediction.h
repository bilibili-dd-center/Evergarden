#pragma once

#include <tensorflow/c/c_api.h>
#include <exception>
#include <string>
#include <memory>

#include "datafeeder.h"

namespace Evergarden::Prediction {
    const auto StatusDeleter = [](TF_Status *status) { TF_DeleteStatus(status); };
    const auto GraphDeleter = [](TF_Graph *g) { TF_DeleteGraph(g); };
    const auto BufferDeleter = [](TF_Buffer *buffer) { TF_DeleteBuffer(buffer); };
    const auto GraphOptsDeleter = [](TF_ImportGraphDefOptions *opts) { TF_DeleteImportGraphDefOptions(opts); };
    const auto SessionOptsDeleter = [](TF_SessionOptions *opts) { TF_DeleteSessionOptions(opts); };

    class bDNNPrediction {
    public:
        bDNNPrediction();
        ~bDNNPrediction();
        void Predict(float *input, int nWindows, int nChannels, int batchSize, float *output);
    private:
        TF_Graph *graph;
        std::unique_ptr<TF_Graph, decltype(GraphDeleter)> graphGuard;
        TF_Tensor *tensor;
    };

    class PredictionException : public std::exception
    {
    public:
        const char* what() const noexcept override
        {
            return message.c_str();
        }

        explicit PredictionException(std::string s): message("[Evergarden::Prediction] " + std::move(s)) { }

    private:
        const std::string message;
    };
}