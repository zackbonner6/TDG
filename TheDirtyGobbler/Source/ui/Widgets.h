#pragma once
// ============================================================================
//  Small reusable controls that bind straight to parameters.
// ============================================================================
#include <juce_audio_processors/juce_audio_processors.h>
#include "LookAndFeel.h"

using APVTS = juce::AudioProcessorValueTreeState;

// Knob with a name on top and the value underneath
class Knob : public juce::Component
{
public:
    Knob (APVTS& state, const juce::String& paramId, const juce::String& name, bool big = false)
        : isBig (big)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, big ? 18 : 15);
        slider.setScrollWheelEnabled (true);
        addAndMakeVisible (slider);

        label.setText (name, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::FontOptions (big ? 15.0f : 12.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, big ? Colours_::text : Colours_::dim);
        label.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label);

        attachment = std::make_unique<APVTS::SliderAttachment> (state, paramId, slider);
        slider.setDoubleClickReturnValue (true, state.getParameter (paramId)->convertFrom0to1 (state.getParameter (paramId)->getDefaultValue()));
    }

    void resized() override
    {
        auto r = getLocalBounds();
        label.setBounds (r.removeFromTop (isBig ? 20 : 15));
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, r.getWidth(), isBig ? 18 : 15);
        slider.setBounds (r);
    }

    juce::Slider slider;

private:
    juce::Label label;
    bool isBig;
    std::unique_ptr<APVTS::SliderAttachment> attachment;
};

// Dropdown with a small caption above it
class ChoiceBox : public juce::Component
{
public:
    ChoiceBox (APVTS& state, const juce::String& paramId, const juce::String& name)
    {
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (state.getParameter (paramId)))
            box.addItemList (c->choices, 1);
        addAndMakeVisible (box);
        label.setText (name, juce::dontSendNotification);
        label.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, Colours_::dim);
        label.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (label);
        attachment = std::make_unique<APVTS::ComboBoxAttachment> (state, paramId, box);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        label.setBounds (r.removeFromTop (15));
        box.setBounds (r.reduced (0, 1));
    }

private:
    juce::ComboBox box;
    juce::Label label;
    std::unique_ptr<APVTS::ComboBoxAttachment> attachment;
};

// Pill-shaped on/off switch
class PowerToggle : public juce::Component
{
public:
    PowerToggle (APVTS& state, const juce::String& paramId, const juce::String& name)
    {
        button.setButtonText (name);
        addAndMakeVisible (button);
        attachment = std::make_unique<APVTS::ButtonAttachment> (state, paramId, button);
    }
    void resized() override { button.setBounds (getLocalBounds()); }
    juce::ToggleButton button;

private:
    std::unique_ptr<APVTS::ButtonAttachment> attachment;
};
