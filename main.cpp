#include <iostream>
#include <fstream>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "pcmio.h"
#include "VAD/mrcg.h"
#include "VAD/datafeeder.h"
#include "VAD/prediction.h"

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
	spdlog::info(format("Started."));
    int len;
    auto test = Evergarden::IO::ReadPCM16LE(std::string("E:\\test.pcm"), &len);
    auto ptr = test.get();
    //int wl = std::atoi(argv[1]);
    auto nE = Evergarden::VAD::MRCG::CalculateNEnvelope(len, 16000, 10);
    auto nW = Evergarden::VAD::MRCG::CalculateNWindows(len, 16000, 10);

    auto allCochleagrams = new float[nW * 64 * 12];
    Evergarden::VAD::MRCG::MRCGDefault(ptr, allCochleagrams, 16000, len);

	Evergarden::VAD::Prediction::VADPrediction pred;
	float* output = new float[nW];
	pred.Predict(allCochleagrams, nW, 768, 128, output);
	spdlog::info(format("Completed"));
    //std::cout << "Wrote"  << std::endl;
    return 0;
}