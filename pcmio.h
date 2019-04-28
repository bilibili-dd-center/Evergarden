#pragma once
#include <string>
#include <memory>

namespace Evergarden::IO
{
	class PCMIOException : public std::exception
	{
	public:
		const char* what() const noexcept override
		{
			return message.c_str();
		}

		explicit PCMIOException(std::string s): message("[Evergarden::IO] " + std::move(s)) { }

	private:
		const std::string message;
	};

	std::unique_ptr<float> ReadPCM16LE(std::string filename, int* nSamplesOut);
}