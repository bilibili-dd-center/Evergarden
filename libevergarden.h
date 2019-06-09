#pragma once

extern "C" {
#ifdef _MSC_VER
	__declspec(dllexport)  long long _stdcall CalculateMRCGSizeDefault(int nSamples, int sampFreq);
	__declspec(dllexport)  void      _stdcall CalculateMRCGDefault(float* matrix, float* output, int sampFreq, int nSamples);
	__declspec(dllexport)  int       _stdcall CalculateMRCGWindowsDefault(int nSamples, int sampFreq);

	__declspec(dllexport)  float* _stdcall ReadPCMFile(const char* name, int* outNSamples);
#else
	__declspec(dllexport) long long CalculateMRCGSizeDefault(int nSamples, int sampFreq) __attribute__((stdcall));
	__declspec(dllexport) void CalculateMRCGDefault(float* matrix, float* output, int sampFreq, int nSamples) __attribute__((stdcall));
	__declspec(dllexport) int CalculateMRCGWindowsDefault(int nSamples, int sampFreq) __attribute__((stdcall));

	__declspec(dllexport) float* ReadPCMFile(const char* name, int* outNSamples) __attribute__((stdcall));
#endif
}