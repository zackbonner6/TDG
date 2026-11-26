#pragma once
// ============================================================================
//  Dark "gobbler" look: charcoal panels, toxic-green accent.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>

namespace Colours_
{
inline const juce::Colour bg      { 0xff0f1113 };
inline const juce::Colour panel   { 0xff181b1f };
inline const juce::Colour panelHi { 0xff22262b };
inline const juce::Colour accent  { 0xffa6e22e };
inline const juce::Colour hot     { 0xffff6b3d };
inline const juce::Colour text    { 0xffe8eaed };
inline const juce::Colour dim     { 0xff7c838d };
inline const juce::Colour track   { 0xff2c3137 };
} // namespace Colours_

class GobblerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GobblerLookAndFeel()
    {
        using namespace juce;
        setColour (Slider::textBoxTextColourId, Colours_::text);
        setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
        setColour (Slider::textBoxBackgroundColourId, Colours::transparentBlack);
        setColour (Label::textColourId, Colours_::text);
        setColour (ComboBox::backgroundColourId, Colours_::panelHi);
        setColour (ComboBox::outlineColourId, Colours_::track);
        setColour (ComboBox::textColourId, Colours_::text);
        setColour (ComboBox::arrowColourId, Colours_::accent);
        setColour (PopupMenu::backgroundColourId, Colours_::panel);
        setColour (PopupMenu::textColourId, Colours_::text);
        setColour (PopupMenu::highlightedBackgroundColourId, Colours_::accent.withAlpha (0.25f));
        setColour (PopupMenu::highlightedTextColourId, Colours_::text);
        setColour (TextButton::buttonColourId, Colours_::panelHi);
        setColour (TextButton::buttonOnColourId, Colours_::accent);
        setColour (TextButton::textColourOffId, Colours_::text);
        setColour (TextButton::textColourOnId, Colours_::bg);
        setColour (AlertWindow::backgroundColourId, Colours_::panel);
        setColour (AlertWindow::textColourId, Colours_::text);
        setColour (TextEditor::backgroundColourId, Colours_::panelHi);
        setColour (TextEditor::textColourId, Colours_::text);
        setColour (TextEditor::outlineColourId, Colours_::track);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float startAngle, float endAngle,
                           juce::Slider& s) override
    {
        using namespace juce;
        auto bounds = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
        const float size = jmin (bounds.getWidth(), bounds.getHeight());
        bounds = bounds.withSizeKeepingCentre (size, size);
        const float stroke = jmax (3.0f, size * 0.08f);
        const float radius = size * 0.5f - stroke * 0.5f;
        const auto c = bounds.getCentre();
        const float angle = startAngle + pos * (endAngle - startAngle);

        // body
        g.setColour (Colours_::panelHi);
        g.fillEllipse (bounds.reduced (stroke * 1.6f));

        // track + value arc
        Path track;
        track.addCentredArc (c.x, c.y, radius, radius, 0.0f, startAngle, endAngle, true);
        g.setColour (Colours_::track);
        g.strokePath (track, PathStrokeType (stroke, PathStrokeType::curved, PathStrokeType::rounded));

        // bipolar knobs (formant, throat, transpose) draw from the centre
        const bool bipolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;
        const float from = bipolar ? (startAngle + endAngle) * 0.5f : startAngle;
        Path val;
        val.addCentredArc (c.x, c.y, radius, radius, 0.0f, jmin (from, angle), jmax (from, angle), true);
        g.setColour (s.isEnabled() ? Colours_::accent : Colours_::dim);
        g.strokePath (val, PathStrokeType (stroke, PathStrokeType::curved, PathStrokeType::rounded));

        // pointer
        const float inner = radius - stroke * 2.2f;
        Point<float> tip (c.x + inner * std::sin (angle), c.y - inner * std::cos (angle));
        g.setColour (Colours_::text);
        g.drawLine ({ c.getPointOnCircumference (inner * 0.35f, angle), tip }, jmax (2.0f, stroke * 0.55f));
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool) override
    {
        using namespace juce;
        auto r = b.getLocalBounds().toFloat().reduced (1.0f);
        const bool on = b.getToggleState();
        g.setColour (on ? Colours_::accent : Colours_::panelHi.brighter (highlighted ? 0.15f : 0.0f));
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (on ? Colours_::bg : Colours_::dim);
        g.setFont (FontOptions (r.getHeight() * 0.58f, Font::bold));
        g.drawText (b.getButtonText(), r, Justification::centred);
    }

    juce::Font getComboBoxFont (juce::ComboBox& box) override
    {
        return juce::FontOptions (juce::jmin (15.0f, (float) box.getHeight() * 0.6f));
    }

    juce::Font getTextButtonFont (juce::TextButton&, int h) override
    {
        return juce::FontOptions (juce::jmin (15.0f, (float) h * 0.55f), juce::Font::bold);
    }
};
