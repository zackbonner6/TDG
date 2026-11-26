#include "PluginEditor.h"

using namespace juce;

// ============================================================================
MainPanel::MainPanel (GobblerProcessor& p)
    : proc (p),
      keyBox (p.apvts, PID::key, "Key"),
      scaleBox (p.apvts, PID::scale, "Scale"),
      inputBox (p.apvts, PID::inputType, "Input Type"),
      tuneToggle (p.apvts, PID::tuneOn, "TUNE"),
      pitch (p),
      retune (p.apvts, PID::retune, "Retune Speed", true),
      humanize (p.apvts, PID::humanize, "Humanize", true),
      formant (p.apvts, PID::formant, "Formant", true),
      mix (p.apvts, PID::mix, "Mix", true),
      flex (p.apvts, PID::flex, "Flex Tune"),
      vibDepth (p.apvts, PID::vibDepth, "Vibrato"),
      vibRate (p.apvts, PID::vibRate, "Vib Rate"),
      throat (p.apvts, PID::throat, "Throat"),
      transpose (p.apvts, PID::transpose, "Transpose"),
      output (p.apvts, PID::outGain, "Output"),
      dbl (p.apvts, PID::dblOn, "DOUBLER"),
      harm (p.apvts, PID::harmOn, "HARMONY"),
      sat (p.apvts, PID::satOn, "SATURATION"),
      deess (p.apvts, PID::dsOn, "DE-ESSER"),
      compP (p.apvts, PID::compOn, "COMPRESSOR"),
      dly (p.apvts, PID::dlyOn, "DELAY"),
      rvb (p.apvts, PID::rvbOn, "REVERB"),
      eq (p.apvts, PID::eqOn, "EQ")
{
    // ---- preset bar ----
    addAndMakeVisible (presetBox);
    presetBox.setTextWhenNothingSelected ("Presets");
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id > 0 && id - 1 != proc.getCurrentPresetIndex())
        {
            proc.loadPreset (id - 1);
            shownPreset = proc.getCurrentPresetIndex();
        }
    };
    for (auto* b : { &prevBtn, &nextBtn, &saveBtn })
        addAndMakeVisible (*b);
    prevBtn.onClick = [this] { stepPreset (-1); };
    nextBtn.onClick = [this] { stepPreset (1); };
    saveBtn.onClick = [this] { showSaveDialog(); };

    // ---- key / scale / notes ----
    for (auto* c : std::initializer_list<Component*> { &keyBox, &scaleBox, &inputBox, &tuneToggle, &pitch })
        addAndMakeVisible (c);
    const auto names = Choices::noteNames();
    for (int i = 0; i < 12; ++i)
    {
        noteBtns[i].setButtonText (names[i]);
        noteBtns[i].setClickingTogglesState (true);
        noteBtns[i].setTooltip ("Click to switch this note off/on");
        addAndMakeVisible (noteBtns[i]);
        noteAtt[i] = std::make_unique<APVTS::ButtonAttachment> (proc.apvts, PID::note (i), noteBtns[i]);
    }

    // ---- knobs ----
    for (auto* k : { &retune, &humanize, &formant, &mix, &flex, &vibDepth, &vibRate, &throat, &transpose, &output })
        addAndMakeVisible (k);

    // ---- effect modules ----
    dbl.addKnob (PID::dblAmount, "Amount");
    dbl.addKnob (PID::dblWidth, "Width");
    harm.addChoice (PID::harmA, "Voice A (left)");
    harm.addChoice (PID::harmB, "Voice B (right)");
    harm.addKnob (PID::harmLevel, "Level");
    harm.addKnob (PID::harmSpread, "Spread");
    sat.addKnob (PID::satDrive, "Drive");
    sat.addKnob (PID::satMix, "Mix");
    deess.addKnob (PID::dsFreq, "Freq");
    deess.addKnob (PID::dsAmount, "Amount");
    compP.addKnob (PID::compThresh, "Thresh");
    compP.addKnob (PID::compRatio, "Ratio");
    compP.addKnob (PID::compMakeup, "Makeup");
    dly.addChoice
