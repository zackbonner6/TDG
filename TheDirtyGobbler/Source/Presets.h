#pragma once
// ============================================================================
//  Factory presets.
//  Values are in real units (ms, %, dB, Hz, semitones). Choices use their
//  index, toggles use 0/1. Anything not listed falls back to the default.
//  Presets never touch Key, Scale, Input Type or the note toggles - those
//  belong to your song and your voice, not the sound.
// ============================================================================
#include <vector>
#include <utility>

struct FactoryPreset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values;
};

// harmony choice index: 0 Off, 1 3rd Up, 2 5th Up, 3 6th Up, 4 Oct Up, 5 3rd Down, 6 4th Down, 7 Oct Down
// delay division index: 0 1/4, 1 1/8, 2 1/8 dotted, 3 1/16, 4 1/4 trip, 5 1/8 trip, 6 1/2
inline const std::vector<FactoryPreset>& getFactoryPresets()
{
    static const std::vector<FactoryPreset> presets = {
        { "Init - Clean Tune", {} },

        { "Hard Snap Trap",
          { { "retune", 0 }, { "humanize", 0 }, { "flex", 0 },
            { "dblOn", 1 }, { "dblAmount", 35 }, { "dblWidth", 45 },
            { "dsOn", 1 }, { "dsAmount", 45 },
            { "compOn", 1 }, { "compThresh", -20 }, { "compRatio", 4 }, { "compMakeup", 5 },
            { "eqOn", 1 }, { "lowCut", 100 }, { "highCut", 16000 },
            { "dlyOn", 1 }, { "dlyDiv", 0 }, { "dlyFb", 20 }, { "dlyMix", 10 },
            { "rvbOn", 1 }, { "rvbSize", 35 }, { "rvbDamp", 50 }, { "rvbMix", 10 } } },

        { "Wavy Melodic",
          { { "retune", 5 }, { "humanize", 10 }, { "vibDepth", 15 }, { "vibRate", 5.2f },
            { "dblOn", 1 }, { "dblAmount", 50 }, { "dblWidth", 60 },
            { "dsOn", 1 }, { "dsAmount", 45 },
            { "compOn", 1 }, { "compThresh", -20 }, { "compRatio", 3.5f }, { "compMakeup", 4 },
            { "eqOn", 1 }, { "lowCut", 100 }, { "highCut", 14000 },
            { "dlyOn", 1 }, { "dlyDiv", 2 }, { "dlyFb", 35 }, { "dlyMix", 22 },
            { "rvbOn", 1 }, { "rvbSize", 65 }, { "rvbDamp", 40 }, { "rvbMix", 25 } } },

        { "Spacey Outer Limits",
          { { "retune", 0 }, { "humanize", 5 }, { "formant", 1 },
            { "dblOn", 1 }, { "dblAmount", 60 }, { "dblWidth", 80 },
            { "harmOn", 1 }, { "harmA", 4 }, { "harmB", 0 }, { "harmLevel", 22 }, { "harmSpread", 80 },
            { "compOn", 1 }, { "compThresh", -22 }, { "compRatio", 4 }, { "compMakeup", 5 },
            { "eqOn", 1 }, { "lowCut", 120 }, { "highCut", 12000 },
            { "dlyOn", 1 }, { "dlyDiv", 0 }, { "dlyFb", 45 }, { "dlyMix", 25 },
            { "rvbOn", 1 }, { "rvbSize", 85 }, { "rvbDamp", 30 }, { "rvbMix", 35 } } },

        { "Natural Polish",
          { { "retune", 60 }, { "humanize", 50 }, { "flex", 40 },
            { "dsOn", 1 }, { "dsAmount", 35 },
            { "compOn", 1 }, { "compThresh", -18 }, { "compRatio", 3 }, { "compMakeup", 3 },
            { "eqOn", 1 }, { "lowCut", 80 }, { "highCut", 18000 },
            { "rvbOn", 1 }, { "rvbSize", 25 }, { "rvbDamp", 50 }, { "rvbMix", 6 } } },

        { "Robot Snap 2007",
          { { "retune", 0 }, { "humanize", 0 }, { "flex", 0 }, { "vibDepth", 0 },
            { "compOn", 1 }, { "compThresh", -20 }, { "compRatio", 4 }, { "compMakeup", 4 },
            { "eqOn", 1 }, { "lowCut", 110 }, { "highCut", 15000 },
            { "dlyOn", 1 }, { "dlyDiv", 1 }, { "dlyFb", 20 }, { "dlyMix", 12 },
            { "rvbOn", 1 }, { "rvbSize", 35 }, { "rvbDamp", 50 }, { "rvbMix", 12 } } },

        { "Rage Distorted",
          { { "retune", 0 }, { "humanize", 0 },
            { "satOn", 1 }, { "satDrive", 70 }, { "satMix", 70 },
            { "dblOn", 1 }, { "dblAmount", 45 }, { "dblWidth", 70 },
            { "dsOn", 1 }, { "dsAmount", 50 },
            { "compOn", 1 }, { "compThresh", -24 }, { "compRatio", 6 }, { "compMakeup", 6 },
            { "eqOn", 1 }, { "lowCut", 150 }, { "highCut", 11000 },
            { "rvbOn", 1 }, { "rvbSize", 40 }, { "rvbDamp", 55 }, { "rvbMix", 12 } } },

        { "Dark Emo Melodic",
          { { "retune", 10 }, { "humanize", 15 }, { "throat", -20 }, { "formant", -1 },
            { "satOn", 1 }, { "satDrive", 25 }, { "satMix", 30 },
            { "dblOn", 1 }, { "dblAmount", 40 }, { "dblWidth", 50 },
            { "compOn", 1 }, { "compThresh", -20 }, { "compRatio", 4 }, { "compMakeup", 4 },
            { "eqOn", 1 }, { "lowCut", 100 }, { "highCut", 10000 },
            { "dlyOn", 1 }, { "dlyDiv", 0 }, { "dlyFb", 40 }, { "dlyMix", 22 },
            { "rvbOn", 1 }, { "rvbSize", 70 }, { "rvbDamp", 60 }, { "rvbMix", 28 } } },

        { "R&B Silk",
          { { "retune", 20 }, { "humanize", 35 }, { "vibDepth", 12 }, { "vibRate", 5.5f },
            { "harmOn", 1 }, { "harmA", 1 }, { "harmB", 0 }, { "harmLevel", 25 }, { "harmSpread", 60 },
            { "dblOn", 1 }, { "dblAmount", 30 }, { "dblWidth", 40 },
            { "dsOn", 1 }, { "dsAmount", 50 },
            { "compOn", 1 }, { "compThresh", -20 }, { "compRatio", 3 }, { "compMakeup", 4 },
            { "eqOn", 1 }, { "lowCut", 90 }, { "highCut", 17000 },
            { "dlyOn", 1 }, { "dlyDiv", 4 }, { "dlyFb", 25 }, { "dlyMix", 12 },
            { "rvbOn", 1 }, { "rvbSize", 60 }, { "rvbDamp", 45 }, { "rvbMix", 20 } } },

        { "Flow Rap Natural",
          { { "retune", 120 }, { "humanize", 60 }, { "flex", 60 },
            { "dblOn", 1 }, { "dblAmount", 15 }, { "dblWidth", 30 },
            { "dsOn", 1 }, { "dsAmount", 45 },
            { "compOn", 1 }, { "compThresh", -22 }, { "compRatio", 4 }, { "compMakeup", 5 },
            { "eqOn", 1 }, { "lowCut", 90 }, { "highCut", 17000 },
            { "rvbOn", 1 }, { "rvbSize", 20 }, { "rvbDamp", 50 }, { "rvbMix", 5 } } },

        { "Chipmunk Pitched Up",
          { { "retune", 10 }, { "humanize", 10 }, { "transpose", 12 }, { "formant", 8 },
            { "dsOn", 1 }, { "dsAmount", 55 },
            { "compOn", 1 }, { "compThresh", -20 }, { "compRatio", 4 }, { "compMakeup", 3 },
            { "eqOn", 1 }, { "lowCut", 150 }, { "highCut", 16000 },
            { "rvbOn", 1 }, { "rvbSize", 30 }, { "rvbDamp", 50 }, { "rvbMix", 8 } } },

        { "Villain Pitched Down",
          { { "retune", 20 }, { "humanize", 20 }, { "transpose", -7 }, { "formant", -4 }, { "throat", -40 },
            { "satOn", 1 }, { "satDrive", 15 }, { "satMix", 25 },
            { "compOn", 1 }, { "compThresh", -22 }, { "compRatio", 4 }, { "compMakeup", 5 },
            { "eqOn", 1 }, { "lowCut", 50 }, { "highCut", 12000 },
            { "dlyOn", 1 }, { "dlyDiv", 0 }, { "dlyFb", 30 }, { "dlyMix", 15 },
            { "rvbOn", 1 }, { "rvbSize", 50 }, { "rvbDamp", 60 }, { "rvbMix", 18 } } },

        { "Phone Lo-Fi Adlib",
          { { "retune", 15 }, { "humanize", 10 },
            { "satOn", 1 }, { "satDrive", 45 }, { "satMix", 60 },
            { "compOn", 1 }, { "compThresh", -24 }, { "compRatio", 6 }, { "compMakeup", 8 },
            { "eqOn", 1 }, { "lowCut", 450 }, { "highCut", 3200 },
            { "rvbOn", 1 }, { "rvbSize", 20 }, { "rvbDamp", 60 }, { "rvbMix", 8 } } },

        { "Stacked Harmony Hook",
          { { "retune", 5 }, { "humanize", 10 },
            { "harmOn", 1 }, { "harmA", 1 }, { "harmB", 7 }, { "harmLevel", 45 }, { "harmSpread", 70 },
            { "dblOn", 1 }, { "dblAmount", 45 }, { "dblWidth", 60 },
            { "dsOn", 1 }, { "dsAmount", 45 },
            { "compOn", 1 }, { "compThresh", -20 }, { "compRatio", 4 }, { "compMakeup", 4 },
            { "eqOn", 1 }, { "lowCut", 100 }, { "highCut", 15000 },
            { "dlyOn", 1 }, { "dlyDiv", 2 }, { "dlyFb", 25 }, { "dlyMix", 12 },
            { "rvbOn", 1 }, { "rvbSize", 55 }, { "rvbDamp", 45 }, { "rvbMix", 18 } } },

        { "Adlib Throw",
          { { "retune", 0 }, { "humanize", 5 },
            { "satOn", 1 }, { "satDrive", 20 }, { "satMix", 30 },
            { "dblOn", 1 }, { "dblAmount", 60 }, { "dblWidth", 90 },
            { "eqOn", 1 }, { "lowCut", 250 }, { "highCut", 9000 },
            { "compOn", 1 }, { "compThresh", -24 }, { "compRatio", 5 }, { "compMakeup", 5 },
            { "dlyOn", 1 }, { "dlyDiv", 0 }, { "dlyFb", 50 }, { "dlyMix", 35 },
            { "rvbOn", 1 }, { "rvbSize", 75 }, { "rvbDamp", 40 }, { "rvbMix", 35 } } },

        { "Pluggnb Bounce",
          { { "retune", 0 }, { "humanize", 5 }, { "formant", 1 },
            { "harmOn", 1 }, { "harmA", 2 }, { "harmB", 0 }, { "harmLevel", 20 }, { "harmSpread", 50 },
            { "dblOn", 1 }, { "dblAmount", 35 }, { "dblWidth", 50 },
            { "dsOn", 1 }, { "dsAmount", 45 },
            { "compOn", 1 }, { "compThresh", -20 }, { "compRatio", 4 }, { "compMakeup", 4 },
            { "eqOn", 1 }, { "lowCut", 110 }, { "highCut", 13000 },
            { "dlyOn", 1 }, { "dlyDiv", 1 }, { "dlyFb", 30 }, { "dlyMix", 18 },
            { "rvbOn", 1 }, { "rvbSize", 60 }, { "rvbDamp", 45 }, { "rvbMix", 22 } } },

        { "Drill Dark Snap",
          { { "retune", 0 }, { "humanize", 5 }, { "throat", -15 },
            { "satOn", 1 }, { "satDrive", 30 }, { "satMix", 30 },
            { "dsOn", 1 }, { "dsAmount", 45 },
            { "compOn", 1 }, { "compThresh", -22 }, { "compRatio", 5 }, { "compMakeup", 6 },
            { "eqOn", 1 }, { "lowCut", 120 }, { "highCut", 12000 },
            { "dlyOn", 1 }, { "dlyDiv", 5 }, { "dlyFb", 25 }, { "dlyMix", 10 },
            { "rvbOn", 1 }, { "rvbSize", 45 }, { "rvbDamp", 65 }, { "rvbMix", 12 } } },

        { "Sing-Rap Glide",
          { { "retune", 90 }, { "humanize", 30 }, { "vibDepth", 10 }, { "vibRate", 5 },
            { "dblOn", 1 }, { "dblAmount", 30 }, { "dblWidth", 40 },
            { "dsOn", 1 }, { "dsAmount", 40 },
            { "compOn", 1 }, { "compThresh", -20 }, { "compRatio", 3.5f }, { "compMakeup", 4 },
            { "eqOn", 1 }, { "lowCut", 90 }, { "highCut", 16000 },
            { "rvbOn", 1 }, { "rvbSize", 45 }, { "rvbDamp", 50 }, { "rvbMix", 15 } } },
    };
    return presets;
}
