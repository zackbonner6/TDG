#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include "Params.h"
#include "dsp/PitchDetector.h"
#include "dsp/PsolaShifter.h"
#include "dsp/ScaleQuantizer.h"
#include "dsp/Effects.h"

class GobblerProcessor : public juce::AudioProcessor
{
public:
    GobblerProcessor();
    ~GobblerProcessor() override = default;

    // ---- AudioProcessor ----
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "TheDirtyGobbler"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    // host "programs" = factory presets
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- parameters ----
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    // ---- presets (factory first, then user presets from disk) ----
    juce::StringArray getPresetNames() const;
    int getNumFactoryPresets() const;
    int getCurrentPresetIndex() const { return currentPreset; }
    void loadPreset (int index);
    bool saveUserPreset (const juce::String& name);
    void refreshUserPresets();
    static juce::File getUserPresetFolder();

    // ---- data for the pitch display (written by audio thread) ----
    std::atomic<float> displayInMidi { -1.0f };
    std::atomic<float> displayOutMidi { -1.0f };

private:
    void runDetection();
    void updateControl (float dt);
    void resetPresetParams();
    void setParam (const juce::String& id, float realValue);

    // --- cached parameter pointers ---
    std::atomic<float>* p (const char* id) { return apvts.getRawParameterValue (id); }
    std::atomic<float> *pKey, *pScale, *pInput, *pTuneOn, *pRetune, *pHumanize, *pFlex, *pVibDepth, *pVibRate,
        *pFormant, *pThroat, *pTranspose, *pNotes[12];
    std::atomic<float> *pDblOn, *pDblAmount, *pDblWidth, *pHarmOn, *pHarmA, *pHarmB, *pHarmLevel, *pHarmSpread,
        *pSatOn, *pSatDrive, *pSatMix, *pDsOn, *pDsFreq, *pDsAmount, *pCompOn, *pCompThresh, *pCompRatio,
        *pCompMakeup, *pDlyOn, *pDlyDiv, *pDlyFb, *pDlyMix, *pRvbOn, *pRvbSize, *pRvbDamp, *pRvbMix, *pEqOn,
        *pLowCut, *pHighCut, *pMix, *pOutGain;

    // --- engine state ---
    double sr = 44100.0;
    int latency = 1024;
    int decim = 2;
    int hopSize = 220, hopCounter = 0;
    int ctrlInterval = 32, ctrlCounter = 0;

    std::vector<float> detRing, detLinear;
    int64_t detWrite = 0;
    int detMask = 0;
    juce::dsp::StateVariableTPTFilter<float> detHP, detLP;
    PitchDetector yin;
    float recentF[3] { 0, 0, 0 };
    int recentIdx = 0;

    bool voiced = false;
    float detectedHz = 0.0f, curPeriod = 0.0f;
    float unvoicedTime = 0.0f, heldTime = 0.0f;
    int targetNote = -1;
    float correction = 0.0f, vibPhase = 0.0f;
    float ratioLead = 1.0f, ratioA = 1.0f, ratioB = 1.0f, formantRatio = 1.0f;
    float harmGate = 0.0f;
    ScaleQuantizer quant;

    PsolaShifter lead, harmVoiceA, harmVoiceB;

    fx::ThroatTilt tilt;
    fx::DeEsser deEss;
    fx::Doubler doubler;
    fx::TempoDelay delay;
    juce::dsp::Compressor<float> comp;
    juce::dsp::StateVariableTPTFilter<float> lowCutF, highCutF;
    juce::Reverb reverb;

    std::vector<float> dryRingL, dryRingR, dryTmpL, dryTmpR;
    int dryPos = 0;
    double bpm = 140.0;

    // presets
    int currentPreset = 0;
    juce::Array<juce::File> userPresetFiles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GobblerProcessor)
};
