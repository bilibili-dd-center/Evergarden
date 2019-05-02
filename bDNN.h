#pragma once

#include <memory>
#include <algorithm>
#include <cstring>

namespace Evergarden::Prediction {
    inline int CalcualtebDNNBlocks(int w, int u) {
        int neighborsLength = (w - u) % u == 0 ? (w - u) / u : (w - u) / u + 1;
        return neighborsLength * 2 + 3;
    }

    void GeneratebDNNNeighbors(int *output, int w, int u);
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
        return (nRows + w * 2) * nCols * nbDNNBlocks;
    }
    void FeedbDNN(data_type *input, int rowPos, int nRows, int nTotalRows, int nCols, data_type *output);
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

    for (i = 0; i < nbDNNBlocks; i++)
        neighbors[i] += w;
    std::reverse(neighbors, neighbors + nbDNNBlocks);
}

template<typename data_type>
void bDNN<data_type>::FeedbDNN(data_type *input, int rowPos, int nRows, int nTotalRows, int nCols, data_type *output) {
    int nOutputColumns = nCols * nbDNNBlocks;
    int rowsLimit = nRows + w * 2;
    std::memset(output, 0, rowsLimit * nOutputColumns * sizeof(data_type));
    data_type *temp = new data_type[nCols];
    for (int row = -w; row < nRows + w; row++) {
        if (rowPos + row < 0) continue;
        for (int ch = 0; ch < nCols; ch++)
            temp[ch] = input[ch * nTotalRows + rowPos + row];
        for (int block = 0; block < nbDNNBlocks; block++) {
            int outputRow = row + neighbors[block];
            if (outputRow < 0 || outputRow >= rowsLimit)
                continue;
            std::memcpy(output + outputRow * nOutputColumns + block * nCols, temp, nCols * sizeof(data_type));
        }
    }
}