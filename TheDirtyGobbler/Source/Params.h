#pragma once
// ============================================================================
//  Parameter IDs + choice lists, shared by processor, editor and presets.
// ============================================================================
#include <juce_core/juce_core.h>

namespace PID
{
// pitch correction
inline constexpr const char* key       = "key";
inline constexpr const char* scale     = "scale";
inline constexpr const char* inputType = "inputType";
inline constexpr const char* tuneOn    = "tuneOn";
inline constexpr const char* retune    = "retune";
inline constexpr const char* humanize  = "humanize";
inline constexpr const char* flex      = "flex";
inline constexpr const char* vibDepth  = "vibDepth";
inline constexpr const char* vibRate   = "vibRate";
inline constexpr const char* formant   = "formant";
inline constexpr const char* throat    = "throat";
inline constexpr const char* transpose = "transpose";
inline juce::String note (int i) { return "note" + juce::String (i); }

// doubler
inline constexpr const char* dblOn     = "dblOn";
inline constexpr const char* dblAmount = "dblAmount";
inline constexpr const char* dblWidth  = "dblWidth";
// harmonizer
inline constexpr const char* harmOn     = "harmOn";
inline constexpr const char* harmA      = "harmA";
inline constexpr const char* harmB      = "harmB";
inline constexpr const char* harmLevel  = "harmLevel";
inline constexpr const char* harmSpread = "harmSpread";
// saturation
inline constexpr const char* satOn    = "satOn";
inline constexpr const char* satDrive = "satDrive";
inline constexpr const char* satMix   = "satMix";
// de-esser
inline constexpr const char* dsOn     = "dsOn";
inline constexpr const char* dsFreq   = "dsFreq";
inline constexpr const char* dsAmount = "dsAmount";
// compressor
inline constexpr const char* compOn     = "compOn";
inline constexpr const char* compThresh = "compThresh";
inline constexpr const char* compRatio  = "compRatio";
inline constexpr const char* compMakeup = "compMakeup";
// delay
inline constexpr const char* dlyOn  = "dlyOn";
inline constexpr const char* dlyDiv = "dlyDiv";
inline constexpr const char* dlyFb  = "dlyFb";
inline constexpr const char* dlyMix = "dlyMix";
// reverb
inline constexpr const char* rvbOn   = "rvbOn";
inline constexpr const char* rvbSize = "rvbSize";
inline constexpr const char* rvbDamp = "rvbDamp";
inline constexpr const char* rvbMix  = "rvbMix";
// eq
inline constexpr const char* eqOn    = "eqOn";
inline constexpr const char* lowCut  = "lowCut";
inline constexpr const char* highCut = "highCut";
// output
inline constexpr const char* mix     = "mix";
inline constexpr const char* outGain = "outGain";
} // namespace PID

namespace Choices
{
inline juce::StringArray noteNames() { return { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }; }
inline juce::StringArray scales()    { return { "Major", "Minor", "Harmonic Minor", "Major Pentatonic", "Minor Pentatonic", "Chromatic" }; }
inline juce::StringArray inputs()    { return { "Low Male", "High Male", "Female" }; }
inline juce::StringArray harmony()   { return { "Off", "3rd Up", "5th Up", "6th Up", "Octave Up", "3rd Down", "4th Down", "Octave Down" }; }
inline juce::StringArray delayDivs() { return { "1/4", "1/8", "1/8 Dotted", "1/16", "1/4 Triplet", "1/8 Triplet", "1/2" }; }
inline float delayBeats (int i)
{
    static const float b[] = { 1.0f, 0.5f, 0.75f, 0.25f, 2.0f / 3.0f, 1.0f / 3.0f, 2.0f };
    return b[juce::jlimit (0, 6, i)];
}
} // namespace Choices
