#include "libevergarden.h"
#include "mrcg.h"
#include "pcmio.h"

void __attribute__((stdcall)) CalculateMRCGDefault(float *matrix, float *output, int sampFreq, int nSamples) {
    Evergarden::MRCG::MRCGDefault(matrix, output, sampFreq, nSamples);
}

int __attribute__((stdcall)) CalculateMRCGWindowsDefault(int nSamples, int sampFreq) {
    return Evergarden::MRCG::CalculateNWindows(nSamples, sampFreq, Evergarden::MRCG::defaultWindowShiftMs);
}

float * __attribute__((stdcall)) ReadPCMFile(const char *name, int *outNSamples) {
    auto ptr = Evergarden::IO::ReadPCM16LE(std::string(name), outNSamples);
    auto res = ptr.get();
    ptr.release();
    return res;
}

long long __attribute__((stdcall)) CalculateMRCGSizeDefault(int nSamples, int sampFreq) {
    return Evergarden::MRCG::CalculateNWindows(nSamples, sampFreq, Evergarden::MRCG::defaultWindowShiftMs) * Evergarden::MRCG::defaultNChannels * 12;
}
