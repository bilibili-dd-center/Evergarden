#include "bDNN.h"

namespace Evergarden::Prediction {
    void GeneratebDNNNeighbors(int *output, int w, int u) {
        int neighborsLength = (w - u) % u == 0 ? (w - u) / u : (w - u) / u + 1;
        int nbDNNBlocks = neighborsLength * 2 + 3;
        int i = 0;
        for (i = 0; i < neighborsLength; i++)
            output[i] = -w + u * i;
        output[i++] = -1;
        output[i++] = 0;
        output[i++] = 1;

        int fnt = neighborsLength + 3;
        for (; i < nbDNNBlocks; i++)
            output[i] = u + 1 + u * (i - fnt);

        for (i = 0; i < nbDNNBlocks; i++)
            output[i] += w;
        std::reverse(output, output + nbDNNBlocks);
    }
}