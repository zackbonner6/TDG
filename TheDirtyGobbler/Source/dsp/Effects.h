#pragma once
// ============================================================================
//  Small hand-rolled effects used in the vocal chain.
//  (Compressor, reverb and the cut filters come from juce::dsp / juce::Reverb.)
// ============================================================================
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>
#include <algorithm>

namespace fx
{
// ---------------------------------------------------------------------------
// Fractional delay line (linear interpolation)
// ---------------------------------------------------------------------------
class DelayLine
{
public:
    void prepare (int maxSamples)
    {
        int size = 1;
        while (size < maxSamples + 4)
            size <<= 1;
        buf.assign ((size_t) size, 0.0f);
        mask = size - 1;
        w = 0;
    }
    void reset() { std::fill (buf.begin(), buf.end(), 0.0f); w = 0; }
    void push (float x) { buf[(size_t) w] = x; w = (w + 1) & mask; }
    float read (float delaySamples) const
    {
        const float d = std::max (1.0f, delaySamples);
        float pos = (float) w - d;
        while (pos < 0.0f)
            pos += (float) buf.size();
        const int i0 = (int) pos;
        const float frac = pos - (float) i0;
        const float a = buf[(size_t) (i0 & mask)];
        const float b = buf[(size_t) ((i0 + 1) & mask)];
        return a + frac * (b - a);
    }

private:
    std::vector<float> buf;
    int mask = 0, w = 0;
};

// ---------------------------------------------------------------------------
// One-pole lowpass
// ---------------------------------------------------------------------------
struct OnePole
{
    float a = 0.1f, z = 0.0f;
    void setCutoff (double sr, double hz) { a = (float) (1.0 - std::exp (-2.0 * juce::MathConstants<double>::pi * hz / sr)); }
    float process (float x) { z += a * (x - z); return z; }
    void reset() { z = 0.0f; }
};

// ---------------------------------------------------------------------------
// Throat tilt: negative = more chest / darker, positive = thinner / brighter
// ---------------------------------------------------------------------------
struct ThroatTilt
{
    OnePole lp;
    void prepare (double sr) { lp.setCutoff (sr, 700.0); lp.reset(); }
    float process (float x, float t) // t in -1..1
    {
        const float lo = lp.process (x);
        const float hi = x - lo;
        return lo * (1.0f - 0.35f * t) + hi * (1.0f + 0.35f * t);
    }
};

// ---------------------------------------------------------------------------
// Saturation (tanh waveshaper with rough level compensation)
// ---------------------------------------------------------------------------
inline float saturate (float x, float drive01, float mix01)
{
    const float g = juce::Decibels::decibelsToGain (drive01 * 36.0f);
    const float wet = std::tanh (g * x + 0.05f * drive01) - std::tanh (0.05f * drive01); // slight asymmetry = warmth
    const float comp = 1.0f / std::sqrt (g);
    return x + mix01 * (wet * comp - x);
}

// ---------------------------------------------------------------------------
// De-esser: splits off the highs, ducks only the highs when they get spiky
// ---------------------------------------------------------------------------
class DeEsser
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        juce::dsp::ProcessSpec spec { sr, 512, 1 };
        hp.prepare (spec);
        hp.setType (juce::dsp::StateVariableTPTFilterType::highpass);
        hp.setResonance (0.707f);
        atk = 1.0f - std::exp (-1.0f / (0.0008f * (float) sr));
        rel = 1.0f - std::exp (-1.0f / (0.060f * (float) sr));
        env = 0.0f;
        gainSm = 1.0f;
    }
    void setParams (float freqHz, float amount01)
    {
        hp.setCutoffFrequency (std::clamp (freqHz, 2000.0f, 12000.0f));
        threshDb = -12.0f - amount01 * 30.0f;
    }
    float process (float x)
    {
        const float h = hp.processSample (0, x);
        const float a = std::abs (h);
        env += (a > env ? atk : rel) * (a - env);
        const float envDb = juce::Decibels::gainToDecibels (env + 1.0e-9f, -120.0f);
        const float over = envDb - threshDb;
        const float targetGain = over > 0.0f ? juce::Decibels::decibelsToGain (-std::min (over * 0.8f, 20.0f)) : 1.0f;
        gainSm += 0.02f * (targetGain - gainSm);
        return x - (1.0f - gainSm) * h;
    }

private:
    juce::dsp::StateVariableTPTFilter<float> hp;
    double sr = 44100.0;
    float atk = 0.1f, rel = 0.001f, env = 0.0f, threshDb = -24.0f, gainSm = 1.0f;
};

// ---------------------------------------------------------------------------
// Doubler: two short, slowly modulated delays panned hard L / R
// ---------------------------------------------------------------------------
class Doubler
{
public:
    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        line.prepare ((int) (0.06 * sampleRate));
        ph1 = 0.0f; ph2 = 1.7f;
    }
    void reset() { line.reset(); }
    void process (float x, float width01, float& outL, float& outR)
    {
        line.push (x);
        const float twoPi = juce::MathConstants<float>::twoPi;
        ph1 += twoPi * 0.31f / sr; if (ph1 > twoPi) ph1 -= twoPi;
        ph2 += twoPi * 0.43f / sr; if (ph2 > twoPi) ph2 -= twoPi;
        const float mod = 0.0004f + 0.0018f * width01;
        const float d1 = (0.010f + 0.004f * width01 + mod * std::sin (ph1)) * sr;
        const float d2 = (0.016f + 0.008f * width01 + mod * std::sin (ph2)) * sr;
        outL = line.read (d1);
        outR = line.read (d2);
    }

private:
    DelayLine line;
    float sr = 44100.0f, ph1 = 0.0f, ph2 = 0.0f;
};

// ---------------------------------------------------------------------------
// Tempo-synced ping-pong delay with darkened repeats
// ---------------------------------------------------------------------------
class TempoDelay
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        lineL.prepare ((int) (2.5 * sr));
        lineR.prepare ((int) (2.5 * sr));
        lpL.setCutoff (sr, 4500.0);
        lpR.setCutoff (sr, 4500.0);
        hpL.setCutoff (sr, 250.0);
        hpR.setCutoff (sr, 250.0);
        cur = (float) (0.25 * sr);
    }
    void reset() { lineL.reset(); lineR.reset(); lpL.reset(); lpR.reset(); hpL.reset(); hpR.reset(); }
    void process (float in, float delaySamples, float fb, float& outL, float& outR)
    {
        cur += 0.0005f * (std::clamp (delaySamples, 1.0f, (float) (2.4 * sr)) - cur);
        const float dl = lineL.read (cur);
        const float dr = lineR.read (cur);
        float fl = lpL.process (dl); fl -= hpL.process (fl);
        float fR = lpR.process (dr); fR -= hpR.process (fR);
        lineL.push (in + fb * fR);
        lineR.push (0.5f * in + fb * fl);
        outL = dl;
        outR = dr;
    }

private:
    DelayLine lineL, lineR;
    OnePole lpL, lpR, hpL, hpR;
    double sr = 44100.0;
    float cur = 10000.0f;
};
} // namespace fx
