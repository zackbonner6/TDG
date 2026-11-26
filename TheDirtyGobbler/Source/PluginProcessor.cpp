#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using namespace juce;

// ============================================================================
//  Parameter layout
// ============================================================================
AudioProcessorValueTreeState::ParameterLayout GobblerProcessor::createLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto addF = [&] (const char* id, const char* name, float lo, float hi, float def, float centre = -1.0f, const char* unit = "")
    {
        NormalisableRange<float> r (lo, hi, 0.0f);
        if (centre > lo && centre < hi)
            r.setSkewForCentre (centre);
        const int decimals = (hi - lo) >= 50.0f ? 0 : 1;
        const String u (unit);
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { id, 1 }, name, r, def,
            AudioParameterFloatAttributes()
                .withLabel (unit)
                .withStringFromValueFunction ([decimals, u] (float v, int) { return String (v, decimals) + (u.isEmpty() ? "" : " " + u); })));
    };
    auto addB = [&] (const juce::String& id, const juce::String& name, bool def)
    { layout.add (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def)); };
    auto addC = [&] (const char* id, const char* name, const StringArray& c, int def)
    { layout.add (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, c, def)); };

    // pitch correction
    addC (PID::key, "Key", Choices::noteNames(), 0);
    addC (PID::scale, "Scale", Choices::scales(), 1);
    addC (PID::inputType, "Input Type", Choices::inputs(), 1);
    addB (PID::tuneOn, "Tune On", true);
    addF (PID::retune, "Retune Speed", 0.0f, 400.0f, 25.0f, 60.0f, "ms");
    addF (PID::humanize, "Humanize", 0.0f, 100.0f, 20.0f, -1, "%");
    addF (PID::flex, "Flex Tune", 0.0f, 100.0f, 0.0f, -1, "cents");
    addF (PID::vibDepth, "Vibrato Depth", 0.0f, 100.0f, 0.0f, -1, "cents");
    addF (PID::vibRate, "Vibrato Rate", 3.0f, 8.0f, 5.5f, -1, "Hz");
    addF (PID::formant, "Formant", -12.0f, 12.0f, 0.0f, -1, "st");
    addF (PID::throat, "Throat", -100.0f, 100.0f, 0.0f, -1, "%");
    layout.add (std::make_unique<AudioParameterInt> (ParameterID { PID::transpose, 1 }, "Transpose", -12, 12, 0,
                                                     AudioParameterIntAttributes().withLabel ("st")));
    const auto names = Choices::noteNames();
    for (int i = 0; i < 12; ++i)
        addB (PID::note (i), "Note " + names[i], true);

    // doubler
    addB (PID::dblOn, "Doubler On", false);
    addF (PID::dblAmount, "Doubler Amount", 0.0f, 100.0f, 40.0f, -1, "%");
    addF (PID::dblWidth, "Doubler Width", 0.0f, 100.0f, 50.0f, -1, "%");
    // harmonizer
    addB (PID::harmOn, "Harmony On", false);
    addC (PID::harmA, "Harmony A", Choices::harmony(), 1);
    addC (PID::harmB, "Harmony B", Choices::harmony(), 0);
    addF (PID::harmLevel, "Harmony Level", 0.0f, 100.0f, 35.0f, -1, "%");
    addF (PID::harmSpread, "Harmony Spread", 0.0f, 100.0f, 60.0f, -1, "%");
    // saturation
    addB (PID::satOn, "Saturation On", false);
    addF (PID::satDrive, "Drive", 0.0f, 100.0f, 20.0f, -1, "%");
    addF (PID::satMix, "Saturation Mix", 0.0f, 100.0f, 50.0f, -1, "%");
    // de-esser
    addB (PID::dsOn, "De-Esser On", false);
    addF (PID::dsFreq, "De-Ess Freq", 3000.0f, 10000.0f, 6500.0f, 6000.0f, "Hz");
    addF (PID::dsAmount, "De-Ess Amount", 0.0f, 100.0f, 40.0f, -1, "%");
    // compressor
    addB (PID::compOn, "Comp On", false);
    addF (PID::compThresh, "Comp Threshold", -40.0f, 0.0f, -18.0f, -1, "dB");
    addF (PID::compRatio, "Comp Ratio", 1.0f, 10.0f, 4.0f, 3.0f, ":1");
    addF (PID::compMakeup, "Comp Makeup", 0.0f, 18.0f, 4.0f, -1, "dB");
    // delay
    addB (PID::dlyOn, "Delay On", false);
    addC (PID::dlyDiv, "Delay Time", Choices::delayDivs(), 1);
    addF (PID::dlyFb, "Delay Feedback", 0.0f, 90.0f, 25.0f, -1, "%");
    addF (PID::dlyMix, "Delay Mix", 0.0f, 100.0f, 15.0f, -1, "%");
    // reverb
    addB (PID::rvbOn, "Reverb On", false);
    addF (PID::rvbSize, "Reverb Size", 0.0f, 100.0f, 40.0f, -1, "%");
    addF (PID::rvbDamp, "Reverb Damping", 0.0f, 100.0f, 50.0f, -1, "%");
    addF (PID::rvbMix, "Reverb Mix", 0.0f, 100.0f, 12.0f, -1, "%");
    // eq
    addB (PID::eqOn, "EQ On", false);
    addF (PID::lowCut, "Low Cut", 20.0f, 1000.0f, 80.0f, 150.0f, "Hz");
    addF (PID::highCut, "High Cut", 2000.0f, 20000.0f, 16000.0f, 8000.0f, "Hz");
    // output
    addF (PID::mix, "Mix", 0.0f, 100.0f, 100.0f, -1, "%");
    addF (PID::outGain, "Output", -24.0f, 12.0f, 0.0f, -1, "dB");

    return layout;
}

// ============================================================================
GobblerProcessor::GobblerProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", AudioChannelSet::stereo(), true)
                          .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    pKey = p (PID::key); pScale = p (PID::scale); pInput = p (PID::inputType); pTuneOn = p (PID::tuneOn);
    pRetune = p (PID::retune); pHumanize = p (PID::humanize); pFlex = p (PID::flex);
    pVibDepth = p (PID::vibDepth); pVibRate = p (PID::vibRate); pFormant = p (PID::formant);
    pThroat = p (PID::throat); pTranspose = p (PID::transpose);
    for (int i = 0; i < 12; ++i)
        pNotes[i] = apvts.getRawParameterValue (PID::note (i));
    pDblOn = p (PID::dblOn); pDblAmount = p (PID::dblAmount); pDblWidth = p (PID::dblWidth);
    pHarmOn = p (PID::harmOn); pHarmA = p (PID::harmA); pHarmB = p (PID::harmB);
    pHarmLevel = p (PID::harmLevel); pHarmSpread = p (PID::harmSpread);
    pSatOn = p (PID::satOn); pSatDrive = p (PID::satDrive); pSatMix = p (PID::satMix);
    pDsOn = p (PID::dsOn); pDsFreq = p (PID::dsFreq); pDsAmount = p (PID::dsAmount);
    pCompOn = p (PID::compOn); pCompThresh = p (PID::compThresh); pCompRatio = p (PID::compRatio); pCompMakeup = p (PID::compMakeup);
    pDlyOn = p (PID::dlyOn); pDlyDiv = p (PID::dlyDiv); pDlyFb = p (PID::dlyFb); pDlyMix = p (PID::dlyMix);
    pRvbOn = p (PID::rvbOn); pRvbSize = p (PID::rvbSize); pRvbDamp = p (PID::rvbDamp); pRvbMix = p (PID::rvbMix);
    pEqOn = p (PID::eqOn); pLowCut = p (PID::lowCut); pHighCut = p (PID::highCut);
    pMix = p (PID::mix); pOutGain = p (PID::outGain);

    refreshUserPresets();
}

bool GobblerProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != AudioChannelSet::mono() && out != AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == out;
}

// ============================================================================
//  prepare
// ============================================================================
void GobblerProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;

    // ~23 ms. Big enough for proper 2-period grains on low voices, small enough to feel tight.
    latency = (int) std::lround (0.0232 * sr);
    setLatencySamples (latency); // tells FL Studio so plugin delay compensation lines everything up

    decim = jlimit (1, 8, (int) std::lround (sr / 22050.0));
    hopSize = jmax (64, (int) std::lround (0.005 * sr));
    ctrlInterval = 32;

    // detection ring: enough for window + max lag at the lowest pitch (55 Hz)
    int size = 1;
    while (size < (int) (4.0 * sr / 55.0) + 64)
        size <<= 1;
    detRing.assign ((size_t) size, 0.0f);
    detMask = size - 1;
    detWrite = 0;
    const int maxLagDec = (int) (sr / 55.0) / decim + 4;
    detLinear.assign ((size_t) (maxLagDec * 2 + 8), 0.0f);
    yin.prepare (maxLagDec + 2);

    dsp::ProcessSpec mono { sr, (uint32) jmax (1, samplesPerBlock), 1 };
    detHP.prepare (mono); detHP.setType (dsp::StateVariableTPTFilterType::highpass); detHP.setCutoffFrequency (50.0f);
    detLP.prepare (mono); detLP.setType (dsp::StateVariableTPTFilterType::lowpass); detLP.setCutoffFrequency (1100.0f);

    lead.prepare (sr, latency);
    harmVoiceA.prepare (sr, latency);
    harmVoiceB.prepare (sr, latency);

    tilt.prepare (sr);
    deEss.prepare (sr);
    doubler.prepare (sr);
    delay.prepare (sr);
    comp.prepare (mono);
    comp.setAttack (4.0f);
    comp.setRelease (90.0f);
    lowCutF.prepare (mono); lowCutF.setType (dsp::StateVariableTPTFilterType::highpass);
    highCutF.prepare (mono); highCutF.setType (dsp::StateVariableTPTFilterType::lowpass);
    reverb.setSampleRate (sr);
    reverb.reset();

    dryRingL.assign ((size_t) latency, 0.0f); // ring of N samples = exactly N samples of delay
    dryRingR.assign ((size_t) latency, 0.0f);
    dryTmpL.assign ((size_t) jmax (1, samplesPerBlock), 0.0f);
    dryTmpR.assign ((size_t) jmax (1, samplesPerBlock), 0.0f);
    dryPos = 0;

    voiced = false; targetNote = -1; correction = 0.0f; heldTime = 0.0f; unvoicedTime = 0.0f;
    harmGate = 0.0f; hopCounter = 0; ctrlCounter = 0;
}

// ============================================================================
//  pitch detection (runs every ~5 ms)
// ============================================================================
void GobblerProcessor::runDetection()
{
    static const float minHz[3] = { 55.0f, 75.0f, 110.0f };
    static const float maxHz[3] = { 600.0f, 800.0f, 1100.0f };
    const int type = jlimit (0, 2, (int) pInput->load());

    const int maxLag = (int) (sr / minHz[type]) / decim + 1;
    const int minLag = jmax (2, (int) (sr / maxHz[type]) / decim);
    const int window = maxLag;
    const int needed = window + maxLag + 2;
    if (needed > (int) detLinear.size())
        return;

    // copy + decimate the newest audio into a linear buffer
    const int64_t start = detWrite - (int64_t) needed * decim;
    float energy = 0.0f;
    for (int j = 0; j < needed; ++j)
    {
        float s = 0.0f;
        for (int k = 0; k < decim; ++k)
            s += detRing[(size_t) ((start + (int64_t) j * decim + k) & detMask)];
        s /= (float) decim;
        detLinear[(size_t) j] = s;
        if (j >= needed - window)
            energy += s * s;
    }
    const float rms = std::sqrt (energy / (float) window);

    const auto res = yin.analyse (detLinear.data(), window, minLag, maxLag, 0.15f);
    const float apThresh = voiced ? 0.35f : 0.25f; // hysteresis: easier to stay voiced than to become voiced
    const bool nowVoiced = rms > 0.0025f && res.aperiodicity < apThresh && res.tau > 1.0f;

    if (nowVoiced)
    {
        const float f = (float) sr / (res.tau * (float) decim);
        recentF[recentIdx] = f;
        recentIdx = (recentIdx + 1) % 3;
        if (! voiced)
            recentF[0] = recentF[1] = recentF[2] = f;
        // median of 3 kills single-frame octave glitches
        float a = recentF[0], b = recentF[1], c = recentF[2];
        const float med = jmax (jmin (a, b), jmin (jmax (a, b), c));
        detectedHz = med;
        curPeriod = (float) sr / med;
        voiced = true;
    }
    else
    {
        voiced = false;
    }
}

// ============================================================================
//  control-rate pitch logic (runs every 32 samples)
// ============================================================================
void GobblerProcessor::updateControl (float dt)
{
    uint16_t mask = 0;
    for (int i = 0; i < 12; ++i)
        if (pNotes[i]->load() > 0.5f)
            mask |= (uint16_t) (1u << i);
    quant.set ((int) pKey->load(), (int) pScale->load(), mask);

    const bool tuneOn = pTuneOn->load() > 0.5f;
    const float transpose = pTranspose->load();
    formantRatio = std::pow (2.0f, (pFormant->load() + pThroat->load() * 0.03f) / 12.0f);

    if (voiced && detectedHz > 20.0f)
    {
        unvoicedTime = 0.0f;
        const float inMidi = 69.0f + 12.0f * std::log2 (detectedHz / 440.0f);

        const int cand = quant.nearest (inMidi);
        if (targetNote < 0 || ! quant.allowed (targetNote))
        {
            targetNote = cand;
            heldTime = 0.0f;
        }
        else if (cand != targetNote && std::abs (inMidi - (float) cand) + 0.2f < std::abs (inMidi - (float) targetNote))
        {
            targetNote = cand; // small hysteresis stops flickering between two notes
            heldTime = 0.0f;
        }
        heldTime += dt;

        float d = (float) targetNote - inMidi;
        const float flex = pFlex->load();
        if (flex > 0.0f)
            d *= jlimit (0.0f, 1.0f, std::abs (d) * 100.0f / flex); // near-pitch notes keep their natural movement
        if (! tuneOn)
            d = 0.0f;

        // retune speed + humanize (humanize slows correction on held notes)
        const float rt = pRetune->load() + (heldTime > 0.12f ? pHumanize->load() * 3.5f : 0.0f);
        const float coef = rt < 1.0f ? 1.0f : 1.0f - std::exp (-dt * 1000.0f / rt);
        correction += (d - correction) * coef;

        // synthetic vibrato, fades in on sustained notes
        float vib = 0.0f;
        const float depth = pVibDepth->load();
        if (depth > 0.0f && heldTime > 0.18f)
        {
            vibPhase += MathConstants<float>::twoPi * pVibRate->load() * dt;
            if (vibPhase > MathConstants<float>::twoPi)
                vibPhase -= MathConstants<float>::twoPi;
            const float env = jmin (1.0f, (heldTime - 0.18f) / 0.25f);
            vib = depth * 0.01f * std::sin (vibPhase) * env;
        }

        const float outMidi = inMidi + correction + vib + transpose;
        ratioLead = std::pow (2.0f, (outMidi - inMidi) / 12.0f);

        // harmonies follow the lead's target note, stay in key
        const int ha = (int) pHarmA->load();
        const int hb = (int) pHarmB->load();
        const float hmA = outMidi + (float) (quant.harmonyNote (targetNote, ha) - targetNote) + 0.06f;
        const float hmB = outMidi + (float) (quant.harmonyNote (targetNote, hb) - targetNote) - 0.06f;
        ratioA = std::pow (2.0f, (hmA - inMidi) / 12.0f);
        ratioB = std::pow (2.0f, (hmB - inMidi) / 12.0f);

        displayInMidi.store (inMidi);
        displayOutMidi.store (outMidi);
    }
    else
    {
        unvoicedTime += dt;
        if (unvoicedTime > 0.08f)
        {
            targetNote = -1;
            heldTime = 0.0f;
            correction = 0.0f;
            displayInMidi.store (-1.0f);
            displayOutMidi.store (-1.0f);
        }
    }
}

// ============================================================================
//  audio
// ============================================================================
void GobblerProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numCh = jmin (2, buffer.getNumChannels());
    if (numCh == 0 || numSamples == 0)
        return;

    if ((int) dryTmpL.size() < numSamples)
    {
        dryTmpL.resize ((size_t) numSamples);
        dryTmpR.resize ((size_t) numSamples);
    }

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                if (*b > 20.0)
                    bpm = *b;

    // ---- block-rate parameter reads ----
    const bool dblOn = pDblOn->load() > 0.5f, harmOn = pHarmOn->load() > 0.5f, satOn = pSatOn->load() > 0.5f;
    const bool dsOn = pDsOn->load() > 0.5f, compOn = pCompOn->load() > 0.5f, dlyOn = pDlyOn->load() > 0.5f;
    const bool rvbOn = pRvbOn->load() > 0.5f, eqOn = pEqOn->load() > 0.5f;
    const float dblAmt = pDblAmount->load() * 0.01f, dblWidth = pDblWidth->load() * 0.01f;
    const float harmLvl = pHarmLevel->load() * 0.01f, spread = pHarmSpread->load() * 0.01f;
    const bool harmAOn = (int) pHarmA->load() != 0, harmBOn = (int) pHarmB->load() != 0;
    const float satDrive = pSatDrive->load() * 0.01f, satMix = pSatMix->load() * 0.01f;
    const float throat = pThroat->load() * 0.01f;
    const float dlySamples = (float) (60.0 / bpm * Choices::delayBeats ((int) pDlyDiv->load()) * sr);
    const float dlyFb = pDlyFb->load() * 0.01f, dlyMix = pDlyMix->load() * 0.01f;
    const float mix = pMix->load() * 0.01f;
    const float outGain = Decibels::decibelsToGain (pOutGain->load());
    const float makeup = Decibels::decibelsToGain (pCompMakeup->load());

    deEss.setParams (pDsFreq->load(), pDsAmount->load() * 0.01f);
    comp.setThreshold (pCompThresh->load());
    comp.setRatio (pCompRatio->load());
    lowCutF.setCutoffFrequency (pLowCut->load());
    highCutF.setCutoffFrequency (jmin (pHighCut->load(), (float) sr * 0.45f));

    // equal-power-ish pans for the two harmony voices
    const float panA = 0.5f - 0.5f * spread, panB = 0.5f + 0.5f * spread;
    const float aL = std::cos (panA * MathConstants<float>::halfPi), aR = std::sin (panA * MathConstants<float>::halfPi);
    const float bL = std::cos (panB * MathConstants<float>::halfPi), bR = std::sin (panB * MathConstants<float>::halfPi);

    float* chL = buffer.getWritePointer (0);
    float* chR = numCh > 1 ? buffer.getWritePointer (1) : nullptr;
    const float ctrlDt = (float) ctrlInterval / (float) sr;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = chL[i];
        const float inR = chR != nullptr ? chR[i] : inL;
        const float x = 0.5f * (inL + inR);

        // dry path delayed by the same latency so dry/wet blends without phasing
        dryTmpL[(size_t) i] = dryRingL[(size_t) dryPos];
        dryTmpR[(size_t) i] = dryRingR[(size_t) dryPos];
        dryRingL[(size_t) dryPos] = inL;
        dryRingR[(size_t) dryPos] = inR;
        dryPos = (dryPos + 1) % (int) dryRingL.size();

        // feed detector
        detRing[(size_t) (detWrite & detMask)] = detLP.processSample (0, detHP.processSample (0, x));
        ++detWrite;
        if (++hopCounter >= hopSize) { hopCounter = 0; runDetection(); }
        if (++ctrlCounter >= ctrlInterval) { ctrlCounter = 0; updateControl (ctrlDt); }

        const float period = voiced ? curPeriod : 0.0f;

        // ---- lead vocal ----
        float v = lead.process (x, period, ratioLead, formantRatio);
        if (std::abs (throat) > 1.0e-4f)
            v = tilt.process (v, throat);
        if (satOn)
            v = fx::saturate (v, satDrive, satMix);
        if (eqOn)
            v = highCutF.processSample (0, lowCutF.processSample (0, v));
        if (dsOn)
            v = deEss.process (v);
        if (compOn)
            v = comp.processSample (0, v) * makeup;

        float L = v, R = v;

        // ---- harmonies (always running so they're in sync when switched on) ----
        const float hA = harmVoiceA.process (x, period, ratioA, formantRatio);
        const float hB = harmVoiceB.process (x, period, ratioB, formantRatio);
        harmGate += 0.002f * ((targetNote >= 0 ? 1.0f : 0.15f) - harmGate);
        if (harmOn)
        {
            float a = harmAOn ? hA * harmLvl * harmGate : 0.0f;
            float b = harmBOn ? hB * harmLvl * harmGate : 0.0f;
            L += a * aL + b * bL;
            R += a * aR + b * bR;
        }

        // ---- doubler ----
        if (dblOn)
        {
            float dl = 0.0f, dr = 0.0f;
            doubler.process (v, dblWidth, dl, dr);
            L += dl * dblAmt;
            R += dr * dblAmt;
        }

        // ---- delay (send style) ----
        if (dlyOn)
        {
            float dl = 0.0f, dr = 0.0f;
            delay.process (0.5f * (L + R), dlySamples, dlyFb, dl, dr);
            L += dl * dlyMix;
            R += dr * dlyMix;
        }

        chL[i] = L;
        if (chR != nullptr)
            chR[i] = R;
    }

    // ---- reverb (block) ----
    if (rvbOn)
    {
        Reverb::Parameters rp;
        rp.roomSize = 0.3f + 0.68f * pRvbSize->load() * 0.01f;
        rp.damping = pRvbDamp->load() * 0.01f;
        rp.wetLevel = pRvbMix->load() * 0.01f * 0.6f;
        rp.dryLevel = 1.0f;
        rp.width = 1.0f;
        reverb.setParameters (rp);
        if (chR != nullptr)
            reverb.processStereo (chL, chR, numSamples);
        else
            reverb.processMono (chL, numSamples);
    }

    // ---- dry/wet + output ----
    for (int i = 0; i < numSamples; ++i)
    {
        chL[i] = (dryTmpL[(size_t) i] * (1.0f - mix) + chL[i] * mix) * outGain;
        if (chR != nullptr)
            chR[i] = (dryTmpR[(size_t) i] * (1.0f - mix) + chR[i] * mix) * outGain;
    }

    for (int ch = numCh; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);
}

// ============================================================================
//  presets
// ============================================================================
int GobblerProcessor::getNumFactoryPresets() const { return (int) getFactoryPresets().size(); }

StringArray GobblerProcessor::getPresetNames() const
{
    StringArray names;
    for (auto& fp : getFactoryPresets())
        names.add (fp.name);
    for (auto& f : userPresetFiles)
        names.add ("User: " + f.getFileNameWithoutExtension());
    return names;
}

File GobblerProcessor::getUserPresetFolder()
{
    return File::getSpecialLocation (File::userDocumentsDirectory).getChildFile ("TheDirtyGobbler").getChildFile ("Presets");
}

void GobblerProcessor::refreshUserPresets()
{
    userPresetFiles.clear();
    auto dir = getUserPresetFolder();
    if (dir.isDirectory())
    {
        userPresetFiles = dir.findChildFiles (File::findFiles, false, "*.xml");
        userPresetFiles.sort();
    }
}

void GobblerProcessor::setParam (const String& id, float realValue)
{
    if (auto* param = apvts.getParameter (id))
        param->setValueNotifyingHost (param->convertTo0to1 (realValue));
}

void GobblerProcessor::resetPresetParams()
{
    // back to defaults for everything a preset controls (not key/scale/input/notes)
    for (auto* ap : getParameters())
    {
        if (auto* rp = dynamic_cast<RangedAudioParameter*> (ap))
        {
            const auto id = rp->getParameterID();
            if (id == PID::key || id == PID::scale || id == PID::inputType || id.startsWith ("note"))
                continue;
            rp->setValueNotifyingHost (rp->getDefaultValue());
        }
    }
}

void GobblerProcessor::loadPreset (int index)
{
    const auto& factory = getFactoryPresets();
    if (index < 0)
        return;

    if (index < (int) factory.size())
    {
        resetPresetParams();
        for (auto& kv : factory[(size_t) index].values)
            setParam (kv.first, kv.second);
        currentPreset = index;
        return;
    }

    const int u = index - (int) factory.size();
    if (u >= userPresetFiles.size())
        return;

    if (auto xml = XmlDocument::parse (userPresetFiles[u]))
    {
        auto tree = ValueTree::fromXml (*xml);
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto child = tree.getChild (i);
            const auto id = child.getProperty ("id").toString();
            if (id == PID::key || id == PID::scale || id == PID::inputType || id.startsWith ("note"))
                continue;
            if (auto* param = apvts.getParameter (id))
                param->setValueNotifyingHost (param->convertTo0to1 ((float) child.getProperty ("value")));
        }
        currentPreset = index;
    }
}

bool GobblerProcessor::saveUserPreset (const String& rawName)
{
    const auto name = File::createLegalFileName (rawName.trim());
    if (name.isEmpty())
        return false;
    auto dir = getUserPresetFolder();
    if (! dir.createDirectory())
        return false;

    auto state = apvts.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    if (xml == nullptr)
        return false;
    const auto file = dir.getChildFile (name + ".xml");
    const bool ok = xml->writeTo (file);
    refreshUserPresets();
    const int idx = userPresetFiles.indexOf (file);
    if (idx >= 0)
        currentPreset = getNumFactoryPresets() + idx;
    return ok;
}

int GobblerProcessor::getNumPrograms() { return getNumFactoryPresets(); }
int GobblerProcessor::getCurrentProgram() { return jmin (currentPreset, getNumFactoryPresets() - 1); }
void GobblerProcessor::setCurrentProgram (int index) { loadPreset (index); }
const String GobblerProcessor::getProgramName (int index)
{
    const auto& f = getFactoryPresets();
    return isPositiveAndBelow (index, (int) f.size()) ? String (f[(size_t) index].name) : String();
}

// ============================================================================
//  state
// ============================================================================
void GobblerProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("presetIndex", currentPreset, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void GobblerProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto tree = ValueTree::fromXml (*xml);
            currentPreset = (int) tree.getProperty ("presetIndex", 0);
            apvts.replaceState (tree);
        }
    }
}

AudioProcessorEditor* GobblerProcessor::createEditor() { return new GobblerEditor (*this); }

// entry point JUCE calls to create the plugin
AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GobblerProcessor(); }
