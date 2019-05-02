#include "pcmio.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <fmt/format.h>
using fmt::format;

#include <spdlog/spdlog.h>
#define LOG_PREFIX "[Evergarden::IO] "

const int PCM_CHUNK_SIZE = 1 * 1024 * 1024; // 1 MB

namespace Evergarden::IO
{
	std::unique_ptr<float> ReadPCM16LE(std::string filename, int* nSamplesOut)
	{
	    spdlog::info(format(LOG_PREFIX "Input: {0}", filename));

        auto path = std::filesystem::path(filename);
	    if (!std::filesystem::exists(path))
	        throw PCMIOException("PCM file not exists!");

	    unsigned long long length;
        try {
            length = std::filesystem::file_size(path);
        } catch (std::ios_base::failure& ex) {
            std::throw_with_nested(PCMIOException("Failed to read from PCM file!"));
        }

		if (length > INT_MAX)
		    throw PCMIOException(format("File is too long! Actual size: {0}; Max Size: {1}", length, INT_MAX));
		if (length % 2 != 0)
		    throw PCMIOException("Malformed PCM file! File must be PCM 16-bit LE in 16000 sample rate and 1 channel!");

		int nSamples = static_cast<int>(length / 2);
		std::unique_ptr<float> res;
		std::unique_ptr<unsigned char> ubuf;
        float *samples;
		unsigned char *buf;
        try {
            res = std::unique_ptr<float>(new float[nSamples]);
            samples = res.get();
            ubuf = std::unique_ptr<unsigned char>(new unsigned char[PCM_CHUNK_SIZE]);
            buf = ubuf.get();
        } catch (std::bad_alloc& ex) {
            std::throw_with_nested(PCMIOException(format("Program has run out of memory! Failed to allocate {0} bytes.", nSamples * 8)));
        }

        try {
            std::ifstream stream;
            std::ios_base::iostate exceptionMask = stream.exceptions() | std::ios::badbit;
            stream.exceptions(exceptionMask);

            stream.open(filename, std::ios::binary);

            if (!stream.good()) throw std::ios_base::failure("Open failed.");

            float *p = samples;
            while (stream) {
                stream.read(reinterpret_cast<char *>(buf), PCM_CHUNK_SIZE);
                unsigned char *end = buf + stream.gcount();
                for (unsigned char *i = buf; i < end; i += 2)
                    *(p++) = (*(reinterpret_cast<short*>(i))) / 32768.0f;
            }

            stream.close();
        } catch (std::ios_base::failure& ex) {
            std::throw_with_nested(PCMIOException("Failed to read from PCM file!"));
        }
        spdlog::info(LOG_PREFIX "Loaded input wave file.");

        *nSamplesOut = (int) nSamples;
        return res;
	}
}
