#include "mrcg.h"

#include <fmt/format.h>

using fmt::format;

#include <spdlog/spdlog.h>

#define LOG_PREFIX "[Evergarden::MRCG] "

namespace Evergarden::MRCG {
    void Envelope(
            const float *input,
            double *envelope,
            const int nSamples,
            const int sampleFreq,
            const int lowCenterFreq,
            const int highCenterFreq,
            const int nChannels,
            const int windowShiftMs) {
        double u0r, u0i;
        double oldcs, oldsn;

        int nPaddedSamples = CalculateNEnvelope(nSamples, sampleFreq, windowShiftMs);

        double lowErb = HzToErbRate(lowCenterFreq);
        double highErb = HzToErbRate(highCenterFreq);
        double spaceErb = (highErb - lowErb) / (nChannels - 1);

        double tpt = 2 * M_PI / sampleFreq;
        //double integrationDecay = exp(-(1000.0 / (sampleFreq * temporalIntegrationMs)));

        //spdlog::info(LOG_PREFIX "Calculating cochleagram envelope...");
        //spdlog::debug(format(LOG_PREFIX "nPaddedSamples={0}", nPaddedSamples));

        for (int channel = 0; channel < nChannels; channel++) {
            double centerFreq = ErbRateToHz(lowErb + channel * spaceErb);
            double tptbw = tpt * erb(centerFreq * BW_CORRECTION);
            double a = exp(-tptbw);
            double gain = fourthPow(tptbw) / 3;
            /*spdlog::debug(
                    format(LOG_PREFIX "Processing channel {0}, cF={1}, tptbw={2}, gain={3}", channel, centerFreq, tptbw,
                    gain));*/

            /* Update filter coefficiants */
            double a1 = 4.0 * a;
            double a2 = -6.0 * square(a);
            double a3 = 4.0 * cube(a);
            double a4 = -fourthPow(a);
            double a5 = square(a);

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

            double coscf = cos(tpt * centerFreq);
            double sincf = sin(tpt * centerFreq);
            double cs = 1;
            double sn = 0;

            double *ptr = envelope + static_cast<long long>(channel) * nPaddedSamples;
            for (int i = 0; i < nSamples; i++) {
                p0r = cs * input[i] + a1 * p1r + a2 * p2r + a3 * p3r + a4 * p4r;
                p0i = sn * input[i] + a1 * p1i + a2 * p2i + a3 * p3i + a4 * p4i;

                /* Clip coefficients to stop them from becoming too close to zero */
                if (fabs(p0r) < VERY_SMALL_NUMBER)
                    p0r = 0.0F;
                if (fabs(p0i) < VERY_SMALL_NUMBER)
                    p0i = 0.0F;

                u0r = p0r + a1 * p1r + a5 * p2r;
                u0i = p0i + a1 * p1i + a5 * p2i;

                p4r = p3r;
                p3r = p2r;
                p2r = p1r;
                p1r = p0r;
                p4i = p3i;
                p3i = p2i;
                p2i = p1i;
                p1i = p0i;

                /*==========================================
                 * Smoothed envelope by temporal integration
                 *==========================================
                 */
                ptr[i] = sqrt(u0r * u0r + u0i * u0i) * gain;
                //senvPrev = ptr[i] = sqrt(u0r * u0r + u0i * u0i) * gain;
                //senvPrev = ptr[i] = sqrt(u0r * u0r + u0i * u0i) * gain + integrationDecay * senvPrev;

                /*=========================================
                 * cs = cos(tpt*i*cf);
                 * sn = -sin(tpt*i*cf);
                 *=========================================
                 */
                cs = (oldcs = cs) * coscf + (oldsn = sn) * sincf;
                sn = oldsn * coscf - oldcs * sincf;
            }

            /*==========================================
             * Padding
             *==========================================
             */
            for (int i = nSamples; i < nPaddedSamples; i++) {
                p0r = a1 * p1r + a2 * p2r + a3 * p3r + a4 * p4r;
                p0i = a1 * p1i + a2 * p2i + a3 * p3i + a4 * p4i;

                u0r = p0r + a1 * p1r + a5 * p2r;
                u0i = p0i + a1 * p1i + a5 * p2i;

                p4r = p3r;
                p3r = p2r;
                p2r = p1r;
                p1r = p0r;
                p4i = p3i;
                p3i = p2i;
                p2i = p1i;
                p1i = p0i;

                /*==========================================
                 * Envelope
                 *==========================================
                 * env0 = sqrt(u0r*u0r+u0i*u0i) * gain;
                 */

                /*==========================================
                 * Smoothed envelope by temporal integration
                 *==========================================
                 */
                //senvPrev = ptr[i] = sqrt(square(u0r) + square(u0i)) * gain + integrationDecay * senvPrev;
                //senvPrev = ptr[i] = sqrt(square(u0r) + square(u0i)) * gain;
                ptr[i] = sqrt(square(u0r) + square(u0i)) * gain;
            }
        }
        //spdlog::info(LOG_PREFIX "Cochleagram envelope calculation completed.");
    }

    void MRCGDefault(const float *input, float *output, int sampleFreq, int nSamples) {
        auto nE = Evergarden::MRCG::CalculateNEnvelope(nSamples, sampleFreq, defaultWindowShiftMs);
        auto nW = Evergarden::MRCG::CalculateNWindows(nSamples, sampleFreq, defaultWindowShiftMs);

        auto envelope = new double[nE * defaultNChannels];
        //auto allCochleagrams = new double[nW * 64 * 12];

        Evergarden::MRCG::Envelope(
                input,
                envelope,
                nSamples,
                sampleFreq,
                50, 8000,
                defaultNChannels,
                defaultWindowShiftMs);
        Evergarden::MRCG::ToWindow<float>(
                envelope,
                output,
                nSamples,
                sampleFreq,
                defaultNChannels,
                defaultWindowLengthMs1,
                defaultWindowShiftMs);
        Evergarden::MRCG::ToWindow<float>(
                envelope,
                output + 64 * nW,
                nSamples,
                sampleFreq,
                defaultNChannels,
                defaultWindowLengthMs2,
                defaultWindowShiftMs);

        delete[] envelope;

        Evergarden::MRCG::Smooth<float>(output, output + defaultNChannels * 2 * nW, nW, defaultNChannels, 5, 5);
        Evergarden::MRCG::Smooth<float>(output, output + defaultNChannels * 3 * nW, nW, defaultNChannels, 11, 11);
        Evergarden::MRCG::Delta<float>(output, nW, defaultNChannels * 4, output + defaultNChannels * 4 * nW, 9);

        auto temp = new float[nW * 64 * 4];

        Evergarden::MRCG::Delta<float>(output, nW, defaultNChannels * 4, temp, 5);
        Evergarden::MRCG::Delta<float>(temp, nW, defaultNChannels * 4, output + defaultNChannels * 8 * nW, 5);

        delete[] temp;
    }
}
