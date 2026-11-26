#pragma once
// ============================================================================
//  PsolaShifter  -  real-time TD-PSOLA pitch shifter with formant control
//
//  How it works (short version):
//   * Analysis: we drop "pitch marks" onto the input, one per pitch period,
//     snapped to the waveform peak so every grain lines up with a glottal pulse.
//   * Synthesis: for every output pitch period we grab the analysis grain
//     closest in time (2 periods long, Hann windowed) and overlap-add it.
//     Spacing the grains closer together raises the pitch, further apart
//     lowers it. Because each grain keeps its own waveform shape, the vocal
//     formants stay put -> no chipmunk effect.
//   * Formant shift: we read each grain slightly faster/slower (resample),
//     which squeezes/stretches the spectral envelope independently of pitch.
//   * Unvoiced sounds (s, t, breaths): a fixed 5 ms grid with no shifting,
//     which reconstructs them transparently.
//
//  Latency is fixed at L samples (the processor reports it to the host).
// ============================================================================
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <array>

class PsolaShifter
{
public:
    void prepare (double sampleRate, int latencySamples)
    {
        sr = sampleRate;
        L = std::max (64, latencySamples);
        int size = 1;
        while (size < L * 8)
            size <<= 1;
        in.assign ((size_t) size, 0.0f);
        out.assign ((size_t) size, 0.0f);
        mask = size - 1;
        unvoicedPeriod = (float) (0.005 * sr);
        reset();
    }

    void reset()
    {
        std::fill (in.begin(), in.end(), 0.0f);
        std::fill (out.begin(), out.end(), 0.0f);
        n = 0;
        lastMark = 0;
        markCount = 0;
        markHead = 0;
        nextSynth = 0.0;
    }

    /** Process one sample.
        periodIn : detected pitch period in samples (<= 1 means unvoiced)
        ratio    : output pitch / input pitch (2.0 = octave up)
        formant  : formant scale (1.0 = untouched, >1 brighter/smaller, <1 deeper/bigger) */
    float process (float x, float periodIn, float ratio, float formant)
    {
        in[(size_t) (n & mask)] = x;

        const bool voiced = periodIn > 1.0f;
        const float P = voiced ? std::clamp (periodIn, 16.0f, (float) L) : unvoicedPeriod;

        // ---------------- analysis pitch marks ----------------
        const int r = voiced ? (int) (P * 0.15f) : 0;
        for (;;)
        {
            const int64_t c = lastMark + (int64_t) std::lround (P);
            if (c + r > n)
                break;
            int64_t best = c;
            if (r > 0)
            {
                float bv = -1.0e9f;
                for (int64_t k = c - r; k <= c + r; ++k)
                {
                    const float v = in[(size_t) (k & mask)];
                    if (v > bv) { bv = v; best = k; }
                }
            }
            pushMark (best, voiced ? P : 0.0f);
            lastMark = best;
        }

        // ---------------- synthesis (overlap-add) ----------------
        const int64_t T = n - L; // output time index for this sample
        const float fr = std::clamp (formant, 0.5f, 2.0f);
        const float hCap = (float) (L - 4) / (1.0f + std::max (fr, 1.0f));

        if (nextSynth < (double) (T + 1))
            nextSynth = (double) (T + 1);

        int guard = 0;
        while (nextSynth <= (double) T + hCap + 1.0 && guard++ < 8)
        {
            const Mark* m = findMark (nextSynth, hCap, fr);
            if (m == nullptr)
            {
                nextSynth += unvoicedPeriod;
                continue;
            }

            const bool mVoiced = m->period > 0.0f;
            const float pm = mVoiced ? m->period : unvoicedPeriod;
            const int H = std::max (8, (int) std::min (pm, hCap));
            float pOut = mVoiced ? pm / std::clamp (ratio, 0.25f, 4.0f) : pm;
            pOut = std::clamp (pOut, 8.0f, 4.0f * hCap);
            const float g = std::min (pOut / (float) H, 2.0f); // keeps loudness constant

            const int64_t s = (int64_t) std::llround (nextSynth);
            const float invH = 3.14159265358979f / (float) H;
            for (int k = -H; k <= H; ++k)
            {
                const int64_t o = s + k;
                if (o < T)
                    continue;
                const float w = 0.5f * (1.0f + std::cos ((float) k * invH));
                const double ip = (double) m->pos + (double) k * fr;
                const int64_t i0 = (int64_t) std::floor (ip);
                const float frac = (float) (ip - (double) i0);
                const float s0 = in[(size_t) (i0 & mask)];
                const float s1 = in[(size_t) ((i0 + 1) & mask)];
                out[(size_t) (o & mask)] += g * w * (s0 + frac * (s1 - s0));
            }
            nextSynth += pOut;
        }

        float y = 0.0f;
        const size_t idx = (size_t) (T & mask);
        if (T >= 0)
            y = out[idx];
        out[idx] = 0.0f;
        ++n;
        return y;
    }

private:
    struct Mark
    {
        int64_t pos = 0;
        float period = 0.0f; // 0 = unvoiced
    };

    void pushMark (int64_t pos, float period)
    {
        marks[(size_t) markHead] = { pos, period };
        markHead = (markHead + 1) % (int) marks.size();
        markCount = std::min (markCount + 1, (int) marks.size());
    }

    // nearest analysis mark to time t whose grain is fully inside the input we have
    const Mark* findMark (double t, float hCap, float fr) const
    {
        const Mark* best = nullptr;
        double bestDist = 1.0e18;
        const int size = (int) marks.size();
        for (int i = 0; i < markCount; ++i)
        {
            const Mark& m = marks[(size_t) ((markHead - 1 - i + size) % size)];
            const float pm = m.period > 0.0f ? m.period : unvoicedPeriod;
            const float H = std::min (pm, hCap);
            if ((double) m.pos + (double) H * fr + 1.0 > (double) n)
                continue; // not enough input yet
            const double d = std::abs ((double) m.pos - t);
            if (d < bestDist) { bestDist = d; best = &m; }
            else if ((double) m.pos < t)
                break; // marks are time ordered, going further back only gets worse
        }
        return best;
    }

    double sr = 44100.0;
    int L = 1024;
    std::vector<float> in, out;
    int64_t mask = 0;
    int64_t n = 0;
    int64_t lastMark = 0;
    std::array<Mark, 256> marks {};
    int markHead = 0, markCount = 0;
    double nextSynth = 0.0;
    float unvoicedPeriod = 220.0f;
};
