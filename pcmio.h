#pragma once
#include <string>
#include <memory>

#include "exceptions.h"

namespace Evergarden::IO
{
	std::unique_ptr<float> ReadPCM16LE(std::string filename, int* nSamplesOut);
}