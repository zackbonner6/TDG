#pragma once
// ============================================================================
//  Scrolling pitch graph: grey = what you sang, green = what comes out.
//  Rows for notes in the current scale are highlighted.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"
#include "LookAndFeel.h"

class PitchDisplay : public juce::Component, private juce::Timer
{
public:
    explicit PitchDisplay (GobblerProcessor& p) : proc (p)
    {
        inHist.fill (-1.0f);
        outHist.fill (-1.0f);
        startTimerHz (40);
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        auto r = getLocalBounds().toFloat();
        g.setColour (Colours_::panel);
        g.fillRoundedRectangle (r, 8.0f);

        auto plot = r.reduced (10.0f, 10.0f).withTrimmedLeft (30.0f);
        const float lo = centre - 9.0f, hi = centre + 9.0f;
        auto yFor = [&] (float midi) { return jmap (midi, lo, hi, plot.getBottom(), plot.getY()); };

        // key/scale rows
        ScaleQuantizer q;
        uint16_t mask = 0;
        for (int i = 0; i < 12; ++i)
            if (proc.apvts.getRawParameterValue (PID::note (i))->load() > 0.5f)
                mask |= (uint16_t) (1u << i);
        q.set ((int) proc.apvts.getRawParameterValue (PID::key)->load(), (int) proc.apvts.getRawParameterValue (PID::scale)->load(), mask);

        const auto names = Choices::noteNames();
        g.setFont (FontOptions (11.0f));
        for (int n = (int) std::ceil (lo); n <= (int) std::floor (hi); ++n)
        {
            const float y = yFor ((float) n);
            const bool on = q.allowed (n);
            g.setColour (on ? Colours_::track.brighter (0.15f) : Colours_::track.withAlpha (0.35f));
            g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
            g.setColour (on ? Colours_::text.withAlpha (0.85f) : Colours_::dim.withAlpha (0.5f));
            g.drawText (names[ScaleQuantizer::pc (n)] + String (n / 12 - 1), Rectangle<float> (r.getX() + 6.0f, y - 7.0f, 34.0f, 14.0f),
                        Justification::centredLeft);
        }

        auto drawLine = [&] (const std::array<float, N>& h, Colour c, float thick)
        {
            Path path;
            bool drawing = false;
            for (int i = 0; i < N; ++i)
            {
                const float v = h[(size_t) ((head + i) % N)];
                const float x = plot.getX() + plot.getWidth() * (float) i / (float) (N - 1);
                if (v < 0.0f || v < lo - 2.0f || v > hi + 2.0f) { drawing = false; continue; }
                const float y = jlimit (plot.getY(), plot.getBottom(), yFor (v));
                if (! drawing) { path.startNewSubPath (x, y); drawing = true; }
                else path.lineTo (x, y);
            }
            g.setColour (c);
            g.strokePath (path, PathStrokeType (thick, PathStrokeType::curved, PathStrokeType::rounded));
        };
        drawLine (inHist, Colours_::dim, 1.6f);
        drawLine (outHist, Colours_::accent, 2.6f);

        // big current-note readout
        const float cur = outHist[(size_t) ((head + N - 1) % N)];
        g.setFont (FontOptions (30.0f, Font::bold));
        g.setColour (cur >= 0.0f ? Colours_::accent : Colours_::dim.withAlpha (0.5f));
        const int nn = (int) std::lround (cur);
        g.drawText (cur >= 0.0f ? names[ScaleQuantizer::pc (nn)] + String (nn / 12 - 1) : String ("--"),
                    r.removeFromTop (44.0f).removeFromRight (110.0f).reduced (8.0f, 4.0f), Justification::centredRight);

        g.setFont (FontOptions (11.0f));
        auto legend = getLocalBounds().toFloat().removeFromBottom (20.0f).removeFromRight (190.0f);
        g.setColour (Colours_::dim);
        g.drawText ("grey = input", legend.removeFromLeft (85.0f), Justification::centred);
        g.setColour (Colours_::accent);
        g.drawText ("green = tuned", legend, Justification::centred);
    }

private:
    void timerCallback() override
    {
        const float in = proc.displayInMidi.load();
        const float out = proc.displayOutMidi.load();
        inHist[(size_t) head] = in;
        outHist[(size_t) head] = out;
        head = (head + 1) % N;
        if (out >= 0.0f)
            centre += 0.08f * (out - centre); // auto-scroll to follow the singer
        repaint();
    }

    static constexpr int N = 200;
    GobblerProcessor& proc;
    std::array<float, N> inHist {}, outHist {};
    int head = 0;
    float centre = 57.0f;
};
