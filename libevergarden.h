#pragma once

extern "C" {
__declspec(dllexport) long long CalculateMRCGSizeDefault(int nSamples, int sampFreq) __attribute__((stdcall));
__declspec(dllexport) void CalculateMRCGDefault(float *matrix, float *output, int sampFreq, int nSamples) __attribute__((stdcall));
__declspec(dllexport) int CalculateMRCGWindowsDefault(int nSamples, int sampFreq) __attribute__((stdcall));

__declspec(dllexport) float* ReadPCMFile(const char* name, int* outNSamples) __attribute__((stdcall));
}