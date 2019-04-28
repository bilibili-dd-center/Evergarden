#include <iostream>
#include <fstream>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "pcmio.h"
#include "mrcg.h"
#include "datafeeder.h"

void write(std::string filename, float* matrix, int rows, int cols) {
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

    auto envelope = new double[nE * 64];
    auto allCochleagrams = new float[nW * 64 * 16];
    auto windows = new double[nW * 64];
    auto windows2 = new double[nW * 64];
    auto smoothed = new double[nW * 64];
    auto smoothed2 = new double[nW * 64];
    Evergarden::MRCG::Envelope(ptr, envelope, len, 16000, 50, 8000, 64, 10);
    Evergarden::MRCG::ToWindow(envelope, allCochleagrams, len, 16000, 64, 25, 10);
    Evergarden::MRCG::ToWindow(envelope, allCochleagrams + 64 * nW, len, 16000, 64, 200, 10);
    Evergarden::MRCG::Smooth(allCochleagrams, allCochleagrams + 64 * 2 * nW, nW, 64, 5, 5);
    Evergarden::MRCG::Smooth(allCochleagrams, allCochleagrams + 64 * 3 * nW, nW, 64, 11, 11);
    Evergarden::MRCG::Delta(allCochleagrams, nW, 64 * 4, allCochleagrams + 64 * 4 * nW, 9);
    Evergarden::MRCG::Delta(allCochleagrams, nW, 64 * 4, allCochleagrams + 64 * 12 * nW, 5);
    Evergarden::MRCG::Delta(allCochleagrams + 64 * 12 * nW, nW, 64 * 4, allCochleagrams + 64 * 8 * nW, 5);

    Evergarden::Prediction::DataFeeder feeder(allCochleagrams, nW, 64 * 12, 128);
    feeder.FeedNext();

    write("E:\\rv.txt", feeder.currentBatch, 64 * feeder.rowLength, feeder.currentBatchSize);

    std::cout << "Wrote"  << std::endl;
    return 0;
}