#include "libevergarden.h"
#include "mrcg.h"
#include "pcmio.h"

#ifdef _MSC_VER 
void _stdcall CalculateMRCGDefault(float* matrix, float* output, int sampFreq, int nSamples) {
#else
void __attribute__((stdcall)) CalculateMRCGDefault(float* matrix, float* output, int sampFreq, int nSamples) {
#endif
    Evergarden::MRCG::MRCGDefault(matrix, output, sampFreq, nSamples);
}

#ifdef _MSC_VER
int _stdcall CalculateMRCGWindowsDefault(int nSamples, int sampFreq) {
#else
int __attribute__((stdcall)) CalculateMRCGWindowsDefault(int nSamples, int sampFreq) {
#endif
    return Evergarden::MRCG::CalculateNWindows(nSamples, sampFreq, Evergarden::MRCG::defaultWindowShiftMs);
}

#ifdef _MSC_VER
float* _stdcall ReadPCMFile(const char* name, int* outNSamples) {
#else
float* __attribute__((stdcall)) ReadPCMFile(const char* name, int* outNSamples) {
#endif
    auto ptr = Evergarden::IO::ReadPCM16LE(std::string(name), outNSamples);
    auto res = ptr.get();
    ptr.release();
    return res;
}

#ifdef _MSC_VER
long long _stdcall CalculateMRCGSizeDefault(int nSamples, int sampFreq) {
#else
long long __attribute__((stdcall)) CalculateMRCGSizeDefault(int nSamples, int sampFreq) {
#endif
    return Evergarden::MRCG::CalculateNWindows(nSamples, sampFreq, Evergarden::MRCG::defaultWindowShiftMs) * Evergarden::MRCG::defaultNChannels * 12;
}
