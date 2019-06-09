#include "datafeeder.h"

#include <fstream>

#include <fmt/format.h>
using namespace fmt;

#include "../exceptions.h"
namespace Evergarden::VAD::Prediction
{
	void ReadFactors(double* output, int len)
	{
		try {
			std::ifstream stream;
			std::ios_base::iostate exceptionMask = stream.exceptions() | std::ios::badbit;
			stream.exceptions(exceptionMask);

			stream.open(normalizeFactorsFilename, std::ios::binary);

			if (!stream.good()) throw std::ios_base::failure("Open failed.");
			stream.read(reinterpret_cast<char*>(output), len * sizeof(double));
			stream.close();
		}
		catch (std::ios_base::failure & ex) {
			std::throw_with_nested(PredictionException(format("Failed to read data normalize factors {0}! The installation may be corrupted!", normalizeFactorsFilename)));
		}
	}
}