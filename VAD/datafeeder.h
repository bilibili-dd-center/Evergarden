#pragma once

#include <memory>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>

#include "bDNN.h"
#include <vector>

namespace Evergarden::VAD::Prediction {
	// DO NOT MODIFY, THIS IS BOUND TO TRAINED MODEL
    const int dw = 19;
    const int du = 9;
	const std::string normalizeFactorsFilename = "normalize_factor.bin";

	inline int CalculatebDNNBlocks(int w, int u) {
		int neighborsLength = (w - u) % u == 0 ? (w - u) / u : (w - u) / u + 1;
		return neighborsLength * 2 + 3;
	}

	void ReadFactors(double* output, int len);
    template <typename data_type>
    class DataFeeder {
    public:
        DataFeeder(data_type *input, int nWindows, int nChannels, int batchSize, int w = dw, int u = du);
        ~DataFeeder();

        void FeedNext();
        bool HasNext() const;

        data_type *currentBatch;
        data_type *currentBatchWithoutPadded;
        int pos, currentBatchSize, currentBatchBufSize;

        int rowLength;
		int w, u;

		int* neighbors;
		int batchSize, nbDNNBlocks;
    private:
        data_type *input, *temp;
		double *factors;
		double *stdDeviation, *mean;

        const int nWindows;
        const int nChannels;

        std::unique_ptr<data_type> batchGuard;
        std::unique_ptr<data_type> tempGuard;
        std::unique_ptr<int> neighborsGuard;
		std::unique_ptr<double> factorsGuard;

        bDNN<data_type> bdnn;
    };

	template<typename data_type>
	double CalculateMean(data_type* input, int len)
	{
		data_type mean = 0;
		for (int i = 0; i < len; i++)
			mean += (input[i] - mean) / (i + 1);
		return mean;
	}

	template<typename data_type>
	double CalculateStd(data_type* input, double mean, int len)
	{
		double var = 0;
		for (int n = 0; n < len; n++)
			var += (input[n] - mean) * (input[n] - mean);
		var /= len;
		return sqrt(var);
	}

    template<typename data_type>
    DataFeeder<data_type>::DataFeeder(data_type *input, int nWindows, int nChannels, int batchSize, int w, int u)
    : input(input), nWindows(nWindows), nChannels(nChannels), batchSize(batchSize), bdnn(bDNN<data_type>(w, u, batchSize)),
    w(w), u(u) {
        batchGuard = std::unique_ptr<data_type>(new data_type[bdnn.CalculatebDNNLength(batchSize, nChannels)]);
        currentBatch = batchGuard.get();
        currentBatchWithoutPadded = currentBatch + bdnn.nbDNNBlocks * nChannels * w;

        pos = 0;
        nbDNNBlocks = CalculatebDNNBlocks(w, u);
        rowLength = nChannels * nbDNNBlocks;

        tempGuard = std::unique_ptr<data_type>(new data_type[nChannels]);
        neighborsGuard = std::unique_ptr<int>(new int[nbDNNBlocks]);
        temp = tempGuard.get();
        neighbors = neighborsGuard.get();

		factors = new double[nChannels * 2];
		factorsGuard = std::unique_ptr<double>(factors);
		stdDeviation = factors + nChannels;
		mean = factors;
		ReadFactors(factors, nChannels * 2);

        GeneratebDNNNeighbors(neighbors, w, u);
    }

    template<typename data_type>
    void DataFeeder<data_type>::FeedNext() {
        currentBatchSize = std::min(nWindows - pos, batchSize);
		currentBatchBufSize = currentBatchSize * rowLength;

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

		for (int row = 0; row < rowsLimit; row++)
			for (int block = 0; block < nbDNNBlocks; block++)
				for (int ch = 0; ch < nChannels; ch++)
					currentBatch[row * rowLength + block * nChannels + ch] = static_cast<float>((currentBatch[row * rowLength + block * nChannels + ch] - mean[ch]) / stdDeviation[ch]);

        pos += currentBatchSize;
    }

    template<typename data_type>
    bool DataFeeder<data_type>::HasNext() const
    {
        return nWindows - pos > 0;
    }

    template<typename data_type>
    DataFeeder<data_type>::~DataFeeder() = default;
}
