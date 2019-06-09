#pragma once
#include <exception>

namespace Evergarden
{
	namespace VAD
	{
		namespace Prediction
		{
			class PredictionException : public std::exception
			{
			public:
				const char* what() const noexcept override
				{
					return message.c_str();
				}

				explicit PredictionException(std::string s) : message("[Evergarden::Prediction] " + std::move(s)) { }

			private:
				const std::string message;
			};

		}
	}

	namespace IO
	{
		class PCMIOException : public std::exception
		{
		public:
			const char* what() const noexcept override
			{
				return message.c_str();
			}

			explicit PCMIOException(std::string s) : message("[Evergarden::IO] " + std::move(s)) { }

		private:
			const std::string message;
		};
	}
}