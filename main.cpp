#include <iostream>
#include <fstream>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "pcmio.h"
#include "mrcg.h"
#include "datafeeder.h"

void writenm(std::string filename, float* matrix, int rows, int cols) {
    std::ofstream out(filename);
    for (int i = 0; i < rows;i++) {
        for (int j = 0;j<cols;j++)
            out<<matrix[i * cols + j]<<' ';
        out<<std::endl;
    }
    out.close();
}

int main(int argc, char** argv) {
    int len;
    auto test = Evergarden::IO::ReadPCM16LE(std::string("E:\\test.pcm"), &len);
    auto ptr = test.get();
    //int wl = std::atoi(argv[1]);
    auto nE = Evergarden::MRCG::CalculateNEnvelope(len, 16000, 10);
    auto nW = Evergarden::MRCG::CalculateNWindows(len, 16000, 10);

    auto allCochleagrams = new float[nW * 64 * 12];
    Evergarden::MRCG::MRCGDefault(ptr, allCochleagrams, 16000, len);

    /*float *finput = new float[153600];
    for (int i = 0; i < 153600; i++)
        finput[i] = i + 1;*/


    //Evergarden::Prediction::DataFeeder feeder(finput, 153600 / 768, 64 * 12, 128);
    Evergarden::Prediction::DataFeeder feeder(allCochleagrams, nW, 64 * 12, 128);
    feeder.FeedNext();
    //feeder.FeedNext();

    writenm("E:\\rv.txt", feeder.currentBatch, feeder.currentBatchSize + 38, feeder.rowLength);

    std::cout << "Wrote"  << std::endl;
    return 0;
}