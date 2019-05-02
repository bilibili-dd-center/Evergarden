#include "prediction.h"

#include <fstream>
#include <filesystem>
#include <fmt/format.h>
using fmt::format;

namespace Evergarden::Prediction {
    const std::string VADModelFilename = "vad_model.pb";

    TF_Buffer* ReadFileToBuffer(std::string filename);

    template<typename T, typename D>
    std::unique_ptr<T, D> make_unique_with_deleter(T* t, D d)
    {
        return std::unique_ptr<T, D> (t, d);
    }

    bDNNPrediction::bDNNPrediction()
    : graph(TF_NewGraph()), graphGuard(make_unique_with_deleter(graph, GraphDeleter)) {
        auto buf = ReadFileToBuffer(VADModelFilename);
        auto bufGuard = make_unique_with_deleter(buf, BufferDeleter);

        auto status = TF_NewStatus();
        auto statusGuard = make_unique_with_deleter(status, StatusDeleter);
        auto opts = TF_NewImportGraphDefOptions();
        auto optsGuard = make_unique_with_deleter(opts, GraphOptsDeleter);
        TF_GraphImportGraphDef(graph, buf, opts, status);
        if (TF_GetCode(status) != TF_OK)
            throw PredictionException(format("Unable to import VAD graph! {0}", TF_Message(status)));
    }

    bDNNPrediction::~bDNNPrediction() {

    }

    void bDNNPrediction::Predict(float *input, int nWindows, int nChannels, int batchSize, float *output) {
        DataFeeder<float> feeder(input, nWindows, nChannels, batchSize);

        auto opts = TF_NewSessionOptions();
        auto optsGuard = make_unique_with_deleter(opts, GraphOptsDeleter);
        auto status = TF_NewStatus();
        auto statusGuard = make_unique_with_deleter(status, StatusDeleter);
        while (feeder.HasNext()) {
            feeder.FeedNext();
            auto session = TF_NewSession(graph, opts, status);
            TF_GraphOperationByName(graph, "");
            TF_SessionRun(session, nullptr, )

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