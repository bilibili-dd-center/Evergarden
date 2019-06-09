#pragma once

#include <cmath>
#include <functional>
#include <memory>

#include <spdlog/spdlog.h>
using namespace fmt;
#define LOG_PREFIX "[Evergarden::MRCG] "

/*=======================
 * Useful Const
 *=======================
 */
#define BW_CORRECTION   1.019
#define VERY_SMALL_NUMBER  1e-200

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

/*=======================
 * Utility functions
 *=======================
 */
#define getMax(x, y)     ((x)>(y)?(x):(y))
#define getRound(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

#define erb(x)          (24.7*(4.37e-3*(x)+1.0))
#define HzToErbRate(x)  (21.4*log10(4.37e-3*(x)+1.0))
#define ErbRateToHz(x)  ((pow(10.0,((x)/21.4))-1.0)/4.37e-3)

#define MRCG_square(x)    ((x) * (x))
#define MRCG_cube(x)      ((x) * (x) * (x))
#define MRCG_fourthPow(x) ((x) * (x)* (x) * (x))

namespace Evergarden::VAD::MRCG {
    const int defaultWindowShiftMs = 10;
    const int defaultWindowLengthMs1 = 10;
    const int defaultWindowLengthMs2 = 10;
    const int defaultNChannels = 64;
	const int defaultResultNChannels = 768;
	const int batchSize = 1024;

    inline int MsToSamples(const int sampleFreq, const int ms) {
        return lround(ms * sampleFreq / 1000);
    }

    inline int64_t CalculateNWindows(const int nSamples, const int sampleFreq, const int windowShiftMs) {
        int windowShiftSamples = MsToSamples(sampleFreq, windowShiftMs);
        return static_cast<int64_t>(ceil(static_cast<long double>(nSamples) / static_cast<long double>(windowShiftSamples)));
    }

    inline int64_t CalculateNEnvelope(const int nSamples, const int sampleFreq, const int windowShiftMs) {
		int64_t nWindows = CalculateNWindows(nSamples, sampleFreq, windowShiftMs);
        return (nWindows * windowShiftMs) * sampleFreq / 1000;
    }

    void MRCGDefault(const float *input, float *output, int sampleFreq, int nSamples);

    void Envelope(
            const float *input,
            double *envelope,
		    int64_t nSamples,
            int sampleFreq,
            int lowCenterFreq,
            int highCenterFreq,
            int nChannels,
            int windowShiftMs);

    /*
    template <typename output_type>
    void ToWindow(
            const double *envelope,
            output_type *output,
            int nSamples,
            int sampleFreq,
            int nChannels,
            int windowLengthMs,
            int windowShiftMs);

    template <typename output_type>
    void Smooth(
            const output_type *windows,
            output_type *output,
            int nWindows,
            int nChannels,
            int vSpan, int hSpan);

    template <typename output_type>
    void Delta(
            output_type *matrix,
            int nColumns,
            int nRows,
            output_type *output,
            int points = 9);*/

    template <typename output_type>
    void ToWindow(
            const double *envelope,
            output_type *output,
            const int64_t nSamples,
            const int sampleFreq,
            const int nChannels,
            const int windowLengthMs,
            const int windowShiftMs) {
		int64_t nWindows = CalculateNWindows(nSamples, sampleFreq, windowShiftMs);
		int64_t nPaddedSamples = CalculateNEnvelope(nSamples, sampleFreq, windowShiftMs);
        int windowShiftSamples = MsToSamples(sampleFreq, windowShiftMs);
        int windowLengthSamples = MsToSamples(sampleFreq, windowLengthMs);
        //double integrationDecay = exp(-(1000.0 / (sampleFreq * temporalIntegrationMs)));

        int increment = windowLengthSamples / windowShiftSamples;

        //spdlog::info(LOG_PREFIX "Summarizing cochleagram into windows...");
        /*spdlog::debug(format(LOG_PREFIX "nWindows={0}, nPaddedSamples={1}, increment={2}", nWindows, nPaddedSamples,
                increment));*/
#pragma omp parallel for
        for (int channel = 0; channel < nChannels; channel++) {
            /*==================================================================================
             * we take the mean of the smoothed envelope as the energy value in each frame
              * rather than simply sampling it.
             * ratemap(c,:) = intgain.*mean(reshape(smoothed_env,frameshift_samples,numframes));
             *==================================================================================
             */
            output_type *outputPos = output + nWindows * channel;
            for (int window = 0; window < nWindows; window++) {
                double sumEnv = 0.0;
                const double *envBegin, *envEnd;

                if (window < increment) {
                    envBegin = envelope;
                    envEnd = envelope + static_cast<long long>(window) * windowShiftSamples;
                } else {
                    envBegin = envelope + static_cast<long long>((channel * nPaddedSamples +
                                                                  (window - increment) * windowShiftSamples));
                    envEnd = envBegin + windowLengthSamples;
                }
                for (auto ptr = envBegin; ptr != envEnd; ptr++) {
                    output_type v = *ptr;
                    sumEnv += v * v;
                }
                /*for (int j = envPos; j < envPos + windowLengthSamples; j++) {
                    sumEnv += envelope[j];
                }*/
                sumEnv += 1e-22;
                output_type result = std::log10(static_cast<output_type>(sumEnv) * 20);
                *(outputPos++) = result;
                //*(outputPos++) = integrationGain * std::log10(sumEnv) * 20 / windowLengthSamples;
            }
        }

        //spdlog::info(LOG_PREFIX "Summarized cochleagram into windows.");
    }

    template <typename output_type>
    void Smooth(const output_type *windows, output_type *output, int nWindows, int nChannels, int vSpan, int hSpan) {
        //spdlog::info(LOG_PREFIX "Smoothing cochleagram features...");

        int fillSize = (2 * vSpan + 1) * (2 * hSpan + 1);
#pragma omp parallel for
        for (int i = 0; i < nChannels; i++) {
            const output_type *rowBegin = windows + static_cast<long long>(nWindows) * std::max(0, i - vSpan);
            const output_type *rowEnd = windows + static_cast<long long>(nWindows) * std::min(nChannels, i + vSpan + 1);
            output_type *outBegin = output + static_cast<long long>(nWindows) * i;

            for (int j = 0; j < nWindows; j++) {
                int colBegin = std::max(0, j - hSpan);
                int colEnd = std::min(nWindows, j + hSpan + 1);

                output_type sum = 0.0f;
                int cnt = 0;
                for (const output_type *k = rowBegin; k < rowEnd; k += nWindows)
                    for (int m = colBegin; m < colEnd; m++, cnt++)
                        sum += *(k + m);
                *(outBegin + j) = sum / fillSize;
            }
        }
        //spdlog::info(LOG_PREFIX "Smoothed cochleagram features.");
    }

    template <typename output_type>
    void Delta(output_type *matrix, int nColumns, int nRows, output_type *output, int points) {
        if (nColumns == 0) {
            std::memcpy(output, matrix, nColumns * nRows * sizeof(output_type));
            return;
        }

        int hLen = floor(points / 2.0);
        points = hLen * 2 + 1;
        output_type *window = new output_type[points];

        output_type windowValue = hLen;
        for (int i = 0; i < points; i++)
            window[i] = windowValue--;

#pragma omp parallel for
        for (int row = 0; row < nRows; row++) {
            output_type *outputRowPtr = output + row * nColumns;
            output_type *inputRowPtr = matrix + row * nColumns;
            for (int i = 0; i < nColumns + hLen; i++) {
                output_type tmp = 0.0f;
                if (i >= hLen) outputRowPtr[i - hLen] = 0.0f;
                for (int j = 0; j < points; j++) {
                    if (i >= nColumns) {
                        tmp += window[j] * inputRowPtr[nColumns - 1];
                        continue;
                    }
                    if (i - j < 0) continue;
                    tmp += window[j] * inputRowPtr[i - j];
                }
                if (i >= hLen)
                    outputRowPtr[i - hLen] = tmp;
            }
        }
    }

	class EnvelopeCalculator
	{
    public:
		EnvelopeCalculator(
			const float* input,
			int64_t nSamples,
			int64_t batchSize,
			int sampleFreq,
			int lowCenterFreq,
			int highCenterFreq,
			int channel,
			int nChannels,
			int windowShiftMs);

		bool HasNext()
		{
			return pos < nPaddedSamples;
		}
		void FeedNext(double *envelope);
	private:
		const float* input;
		int64_t nSamples, nPaddedSamples;
		int64_t pos = 0;
		int sampleFreq, channel, batchSize;

		double lowErb, highErb, spaceErb, tptbw, a, gain, centerFreq;
		double a1;
		double a2;
		double a3;
		double a4;
		double a5;

		double p0r = 0.0;
		double p1r = 0.0;
		double p2r = 0.0;
		double p3r = 0.0;
		double p4r = 0.0;

		double p0i = 0.0;
		double p1i = 0.0;
		double p2i = 0.0;
		double p3i = 0.0;
		double p4i = 0.0;

		double coscf, sincf;
		double oldcs, oldsn;
		double u0r, u0i;
		double cs = 1;
		double sn = 0;
	};
}