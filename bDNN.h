#pragma once

#include <memory>
#include <algorithm>

namespace Evergarden::Prediction {
    const int bDNNLength = 7;
    const int bDNNNeighbor[] = { -19, -10, -1, 0, 1, 10, 19 };

    inline int CalculatebDNNLength(int nRows, int nCols) {
        return nRows * nCols * bDNNLength;
    }

    template <typename data_type>
    void bDNNTransform(data_type *input, int nRows, int nCols, data_type *output) {
        int length = CalculatebDNNLength(nRows, nCols);
        memset(output, 0, length * sizeof(data_type));

    }
}
template <typename data_type>
class bDNN {
private:
    int *neighbors;

    int w, u, batchSize;

    std::unique_ptr<int> neighborsGuard;

public:
    int nbDNNBlocks;

    bDNN(int w, int u, int batchSize);
    inline int CalculatebDNNLength(int nRows, int nCols) {
        return nRows * nCols * nbDNNBlocks;
    }
    void FeedbDNN(data_type *input, int rowPos, int nRows, int nCols, data_type *output);
};

template<typename data_type>
bDNN<data_type>::bDNN(int w, int u, int batchSize) : w(w), u(u), batchSize(batchSize) {
    // fuck my life.
    int neighborsLength = (w - u) % u == 0 ? (w - u) / u : (w - u) / u + 1;
    nbDNNBlocks = neighborsLength * 2 + 3;
    neighborsGuard = std::unique_ptr<int>(new int[nbDNNBlocks]);
    neighbors = neighborsGuard.get();
    int i = 0;
    for (i = 0; i < neighborsLength; i++)
        neighbors[i] = -w + u * i;
    neighbors[i++] = -1;
    neighbors[i++] = 0;
    neighbors[i++] = 1;

    int fnt = neighborsLength + 3;
    for (; i < nbDNNBlocks; i++)
        neighbors[i] = u + 1 + u * (i - fnt);

    auto m = *std::min_element(neighbors, neighbors + nbDNNBlocks);
    for (i = 0; i < nbDNNBlocks; i++)
        neighbors[i] += m;
    std::reverse(neighbors, neighbors + nbDNNBlocks);
}

template<typename data_type>
void bDNN<data_type>::FeedbDNN(data_type *input, int rowPos, int nRows, int nCols, data_type *output) {
    int outputCols = nCols * nbDNNBlocks;

    for (int i = 0; i < nRows; i++) {
        for (int n = 0; n < nbDNNBlocks; n++) {
            for (int c = 0; c < nCols; c++) {
                output[i * outputCols + n * nCols + c] = input[c * nRows + rowPos + i + neighbors[n]];
            }
        }
    }
}