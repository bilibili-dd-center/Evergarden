#pragma once

#include <memory>
#include <cstdlib>

#include "bDNN.h"

namespace Evergarden::Prediction {
    const int w = 19;
    const int u = 9;

    template <typename data_type>
    class DataFeeder {
    public:
        DataFeeder(data_type *input, int nWindows, int nRows, int batchSize);
        ~DataFeeder();

        void FeedNext();

        data_type *currentBatch;
        int currentBatchSize;

        int rowLength;
    private:
        data_type *input;

        data_type *padded;

        int nWindows;
        int nRows;
        int batchSize;

        std::unique_ptr<data_type> paddedGuard;
        std::unique_ptr<data_type> batchGuard;

        bDNN<data_type> bdnn;

        int pos;
    };

    template<typename data_type>
    DataFeeder<data_type>::DataFeeder(data_type *input, int nWindows, int nRows, int batchSize)
    : input(input), nWindows(nWindows), nRows(nRows), batchSize(batchSize), bdnn(bDNN<data_type>(w, u, batchSize)) {
        int batchPadding = batchSize - nWindows % batchSize;
        int windowPadding = w;
        int resultRows = nWindows + windowPadding * 2 + batchPadding;

        paddedGuard = std::unique_ptr<data_type>(new data_type[resultRows * static_cast<unsigned long long>(nRows)]);
        padded = paddedGuard.get();

        data_type *target = padded;
        for (int i = 0; i < nRows; i++) {
            memset(target, 0, windowPadding * sizeof(data_type));
            memcpy(target + windowPadding, input + i * nWindows, nWindows * sizeof(data_type));
            memset(target + windowPadding + nWindows, 0, (windowPadding + batchPadding) * sizeof(data_type));
            target += resultRows;
        }

        batchGuard = std::unique_ptr<data_type>(new data_type[bdnn.CalculatebDNNLength(resultRows, nRows)]);
        currentBatch = batchGuard.get();

        pos = w;
        rowLength = nRows * bdnn.nbDNNBlocks;
    }

    template<typename data_type>
    void DataFeeder<data_type>::FeedNext() {
        currentBatchSize = std::min(nWindows - pos, batchSize);
        pos += currentBatchSize;
        bdnn.FeedbDNN(padded, pos, currentBatchSize, nRows, currentBatch);
    }

    template<typename data_type>
    DataFeeder<data_type>::~DataFeeder() {

    }
}