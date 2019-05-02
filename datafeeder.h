#pragma once

#include <memory>
#include <cstdlib>

#include "bDNN.h"

namespace Evergarden::Prediction {
    const int dw = 19;
    const int du = 9;

    template <typename data_type>
    class DataFeeder {
    public:
        DataFeeder(data_type *input, int nWindows, int nChannels, int batchSize, int w = dw, int u = du);
        ~DataFeeder();

        void FeedNext();
        bool HasNext();

        data_type *currentBatch;
        data_type *currentBatchWithoutPadded;
        int currentBatchSize;

        int rowLength;
    private:
        data_type *input, *temp;
        int *neighbors;

        int nWindows;
        int nChannels;
        int batchSize, nbDNNBlocks;

        std::unique_ptr<data_type> batchGuard;
        std::unique_ptr<data_type> tempGuard;
        std::unique_ptr<int> neighborsGuard;

        bDNN<data_type> bdnn;

        int pos;
        int w, u;
    };

    template<typename data_type>
    DataFeeder<data_type>::DataFeeder(data_type *input, int nWindows, int nChannels, int batchSize, int w, int u)
    : input(input), nWindows(nWindows), nChannels(nChannels), batchSize(batchSize), bdnn(bDNN<data_type>(w, u, batchSize)),
    w(w), u(u) {

        batchGuard = std::unique_ptr<data_type>(new data_type[bdnn.CalculatebDNNLength(batchSize, nChannels)]);
        currentBatch = batchGuard.get();
        currentBatchWithoutPadded = currentBatch + bdnn.nbDNNBlocks * nChannels * w;

        pos = 0;
        nbDNNBlocks = CalcualtebDNNBlocks(w, u);
        rowLength = nChannels * nbDNNBlocks;

        tempGuard = std::unique_ptr<data_type>(new data_type[nChannels]);
        neighborsGuard = std::unique_ptr<int>(new int[nbDNNBlocks]);
        temp = tempGuard.get();
        neighbors = neighborsGuard.get();

        GeneratebDNNNeighbors(neighbors, w, u);
    }

    template<typename data_type>
    void DataFeeder<data_type>::FeedNext() {
        currentBatchSize = std::min(nWindows - pos, batchSize);

        int nOutputColumns = nChannels * nbDNNBlocks;
        int rowsLimit = batchSize + w * 2;
        std::memset(currentBatch, 0, rowsLimit * nOutputColumns * sizeof(data_type));
        for (int row = -w; row < batchSize + w; row++) {
            if (pos + row < 0) continue;
            for (int ch = 0; ch < nChannels; ch++)
                temp[ch] = input[ch * nWindows + pos + row];
            for (int block = 0; block < nbDNNBlocks; block++) {
                int outputRow = row + neighbors[block];
                if (outputRow < 0 || outputRow >= rowsLimit)
                    continue;
                std::memcpy(currentBatch + outputRow * nOutputColumns + block * nChannels, temp, nChannels * sizeof(data_type));
            }
        }

        pos += currentBatchSize;
    }

    template<typename data_type>
    bool DataFeeder<data_type>::HasNext() {
        return nWindows - pos > 0;
    }

    template<typename data_type>
    DataFeeder<data_type>::~DataFeeder() {

    }
}