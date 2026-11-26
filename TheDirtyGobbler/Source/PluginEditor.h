#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/LookAndFeel.h"
#include "ui/Widgets.h"
#include "ui/PitchDisplay.h"

// One effect module box: power switch on top, dropdowns stacked on the left, knobs on the right
class FxPanel : public juce::Component
{
public:
    FxPanel (APVTS& s, const juce::String& toggleId, const juce::String& title) : state (s), toggle (s, toggleId, title)
    {
        addAndMakeVisible (toggle);
    }
    void addKnob (const juce::String& id, const juce::String& name)
    {
        knobs.push_back (std::make_unique<Knob> (state, id, name));
        addAndMakeVisible (*knobs.back());
    }
    void addChoice (const juce::String& id, const juce::String& name)
    {
        choices.push_back (std::make_unique<ChoiceBox> (state, id, name));
        addAndMakeVisible (*choices.back());
    }
    void paint (juce::Graphics& g) override
    {
        g.setColour (Colours_::panel);
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 8.0f);
    }
    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        toggle.setBounds (r.removeFromTop (22).removeFromLeft (130));
        r.removeFromTop (4);
        if (! choices.empty())
        {
            auto col = r.removeFromLeft (104);
            const int h = juce::jmin (44, col.getHeight() / (int) choices.size());
            col = col.withSizeKeepingCentre (col.getWidth(), h * (int) choices.size());
            for (auto& c : choices)
                c->setBounds (col.removeFromTop (h).reduced (0, 2));
            r.removeFromLeft (4);
        }
        if (! knobs.empty())
        {
            const int w = r.getWidth() / (int) knobs.size();
            for (auto& k : knobs)
                k->setBounds (r.removeFromLeft (w));
        }
    }

private:
    APVTS& state;
    PowerToggle toggle;
    std::vector<std::unique_ptr<Knob>> knobs;
    std::vector<std::unique_ptr<ChoiceBox>> choices;
};

// Everything, laid out at a fixed 1000 x 700 and scaled as a whole when the window is resized
class MainPanel : public juce::Component, private juce::Timer
{
public:
    explicit MainPanel (GobblerProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshPresetBox();
    void stepPreset (int delta);
    void showSaveDialog();

    GobblerProcessor& proc;

    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, saveBtn { "Save" };

    ChoiceBox keyBox, scaleBox, inputBox;
    PowerToggle tuneToggle;
    juce::TextButton noteBtns[12];
    std::unique_ptr<APVTS::ButtonAttachment> noteAtt[12];

    PitchDisplay pitch;

    Knob retune, humanize, formant, mix;
    Knob flex, vibDepth, vibRate, throat, transpose, output;

    FxPanel dbl, harm, sat, deess, compP, dly, rvb, eq;

    std::unique_ptr<juce::AlertWindow> saveDialog;
    int shownPreset = -2;
    int shownPresetCount = -1;
};

class GobblerEditor : public juce::AudioProcessorEditor
{
public:
    explicit GobblerEditor (GobblerProcessor&);
    ~GobblerEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

    static constexpr int baseW = 1000, baseH = 700;

private:
    GobblerLookAndFeel lnf;
    MainPanel content;
};
