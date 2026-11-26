#pragma once
// ============================================================================
//  PitchDetector  -  YIN monophonic pitch detection
//  (de Cheveigne & Kawahara, 2002)
//
//  The processor feeds us a short, already-decimated chunk of audio and we
//  return the best period (in decimated samples) plus an "aperiodicity" score:
//  0 = perfectly periodic (clearly sung note), 1 = noise (consonants, breath).
// ============================================================================
#include <vector>
#include <cmath>
#include <algorithm>

class PitchDetector
{
public:
    struct Result
    {
        float tau = 0.0f;          // period in samples (fractional)
        float aperiodicity = 1.0f; // YIN CMND value at the chosen lag
    };

    void prepare (int maxLag)
    {
        diff.assign ((size_t) maxLag + 2, 0.0f);
        cmnd.assign ((size_t) maxLag + 2, 1.0f);
    }

    // x must hold at least (window + maxLag + 1) samples.
    Result analyse (const float* x, int window, int minLag, int maxLag, float threshold = 0.15f)
    {
        Result r;
        if (maxLag + 1 > (int) diff.size() || minLag < 2 || maxLag <= minLag + 2)
            return r;

        // 1) difference function d(tau)
        diff[0] = 0.0f;
        for (int tau = 1; tau <= maxLag; ++tau)
        {
            float s = 0.0f;
            const float* a = x;
            const float* b = x + tau;
            for (int j = 0; j < window; ++j)
            {
                const float d = a[j] - b[j];
                s += d * d;
            }
            diff[(size_t) tau] = s;
        }

        // 2) cumulative mean normalised difference
        cmnd[0] = 1.0f;
        float running = 0.0f;
        for (int tau = 1; tau <= maxLag; ++tau)
        {
            running += diff[(size_t) tau];
            cmnd[(size_t) tau] = running > 1.0e-12f ? diff[(size_t) tau] * (float) tau / running : 1.0f;
        }

        // 3) absolute threshold -> first dip below threshold, then walk to its minimum
        int best = -1;
        for (int tau = minLag; tau < maxLag; ++tau)
        {
            if (cmnd[(size_t) tau] < threshold)
            {
                while (tau + 1 < maxLag && cmnd[(size_t) tau + 1] < cmnd[(size_t) tau])
                    ++tau;
                best = tau;
                break;
            }
        }

        // no dip below threshold: fall back to the global minimum
        if (best < 0)
        {
            best = minLag;
            for (int tau = minLag + 1; tau < maxLag; ++tau)
                if (cmnd[(size_t) tau] < cmnd[(size_t) best])
                    best = tau;
        }

        // 4) parabolic interpolation for sub-sample accuracy
        float t = (float) best;
        if (best > 1 && best < maxLag)
        {
            const float a = cmnd[(size_t) best - 1];
            const float b = cmnd[(size_t) best];
            const float c = cmnd[(size_t) best + 1];
            const float denom = a - 2.0f * b + c;
            if (std::abs (denom) > 1.0e-9f)
                t = (float) best + 0.5f * (a - c) / denom;
        }

        r.tau = t;
        r.aperiodicity = cmnd[(size_t) best];
        return r;
    }

private:
    std::vector<float> diff, cmnd;
};
