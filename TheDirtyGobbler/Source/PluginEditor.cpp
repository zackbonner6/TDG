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
    dly.addChoice (PID::dlyDiv, "Time (synced)");
    dly.addKnob (PID::dlyFb, "Feedback");
    dly.addKnob (PID::dlyMix, "Mix");
    rvb.addKnob (PID::rvbSize, "Size");
    rvb.addKnob (PID::rvbDamp, "Damp");
    rvb.addKnob (PID::rvbMix, "Mix");
    eq.addKnob (PID::lowCut, "Low Cut");
    eq.addKnob (PID::highCut, "High Cut");
    for (auto* f : { &dbl, &harm, &sat, &deess, &compP, &dly, &rvb, &eq })
        addAndMakeVisible (f);

    refreshPresetBox();
    startTimerHz (10);
}

void MainPanel::paint (Graphics& g)
{
    g.fillAll (Colours_::bg);

    // title
    g.setColour (Colours_::accent);
    g.setFont (FontOptions (26.0f, Font::bold));
    g.drawText ("THE DIRTY GOBBLER", 20, 10, 330, 34, Justification::centredLeft);
    g.setColour (Colours_::dim);
    g.setFont (FontOptions (11.0f));
    g.drawText ("rap vocal tuner  |  " + String (proc.getLatencySamples()) + " samples latency", 20, 38, 330, 14, Justification::centredLeft);

    g.setFont (FontOptions (12.0f, Font::bold));
    g.drawText ("Notes (click to turn off)", 510, 58, 300, 15, Justification::centredLeft);

    // right-hand knob area background
    g.setColour (Colours_::panel);
    g.fillRoundedRectangle (600.0f, 110.0f, 380.0f, 225.0f, 8.0f);
}

void MainPanel::resized()
{
    // header
    prevBtn.setBounds (430, 14, 30, 28);
    presetBox.setBounds (464, 14, 300, 28);
    nextBtn.setBounds (768, 14, 30, 28);
    saveBtn.setBounds (808, 14, 80, 28);

    // key row
    keyBox.setBounds (20, 58, 80, 44);
    scaleBox.setBounds (110, 58, 160, 44);
    inputBox.setBounds (280, 58, 120, 44);
    tuneToggle.setBounds (412, 76, 80, 24);
    for (int i = 0; i < 12; ++i)
        noteBtns[i].setBounds (510 + i * 39, 76, 36, 25);

    // pitch display
    pitch.setBounds (20, 110, 565, 225);

    // big knobs
    const int bw = 95;
    int x = 600;
    for (auto* k : { &retune, &humanize, &formant, &mix })
    {
        k->setBounds (x, 116, bw, 122);
        x += bw;
    }
    // small knobs
    const int sw = 63;
    x = 601;
    for (auto* k : { &flex, &vibDepth, &vibRate, &throat, &transpose, &output })
    {
        k->setBounds (x, 246, sw, 84);
        x += sw;
    }

    // effects grid
    const int gx = 20, gy = 347, gap = 10;
    const int pw = (960 - 3 * gap) / 4, ph = (700 - gy - 12 - gap) / 2;
    FxPanel* panels[8] = { &dbl, &harm, &sat, &deess, &compP, &dly, &rvb, &eq };
    for (int i = 0; i < 8; ++i)
        panels[i]->setBounds (gx + (i % 4) * (pw + gap), gy + (i / 4) * (ph + gap), pw, ph);
}

void MainPanel::timerCallback()
{
    // colour note buttons: bright = in scale, dull = outside the current scale
    ScaleQuantizer q;
    q.set ((int) proc.apvts.getRawParameterValue (PID::key)->load(), (int) proc.apvts.getRawParameterValue (PID::scale)->load(), 0xFFF);
    for (int i = 0; i < 12; ++i)
    {
        const bool in = q.inScale (i);
        noteBtns[i].setColour (TextButton::buttonOnColourId, in ? Colours_::accent : Colours_::dim.darker (0.3f));
        noteBtns[i].setColour (TextButton::textColourOnId, in ? Colours_::bg : Colours_::text.withAlpha (0.6f));
        noteBtns[i].setColour (TextButton::textColourOffId, Colours_::dim.withAlpha (0.6f));
        noteBtns[i].setColour (TextButton::buttonColourId, Colours_::bg);
    }

    // keep preset box in sync if the host changed program
    if (proc.getCurrentPresetIndex() != shownPreset || proc.getPresetNames().size() != shownPresetCount)
        refreshPresetBox();
}

void MainPanel::refreshPresetBox()
{
    const auto names = proc.getPresetNames();
    if (names.size() != shownPresetCount)
    {
        presetBox.clear (dontSendNotification);
        const int nf = proc.getNumFactoryPresets();
        presetBox.addSectionHeading ("Factory");
        for (int i = 0; i < names.size(); ++i)
        {
            if (i == nf)
                presetBox.addSectionHeading ("User");
            presetBox.addItem (names[i], i + 1);
        }
        shownPresetCount = names.size();
    }
    presetBox.setSelectedId (proc.getCurrentPresetIndex() + 1, dontSendNotification);
    shownPreset = proc.getCurrentPresetIndex();
}

void MainPanel::stepPreset (int delta)
{
    const int n = proc.getPresetNames().size();
    if (n == 0)
        return;
    const int next = (proc.getCurrentPresetIndex() + delta + n) % n;
    proc.loadPreset (next);
    refreshPresetBox();
}

void MainPanel::showSaveDialog()
{
    saveDialog = std::make_unique<AlertWindow> ("Save Preset", "Name your preset:", MessageBoxIconType::NoIcon, this);
    saveDialog->addTextEditor ("name", "My Gobble");
    saveDialog->addButton ("Save", 1, KeyPress (KeyPress::returnKey));
    saveDialog->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
    saveDialog->enterModalState (true, ModalCallbackFunction::create ([this] (int result)
    {
        if (result == 1 && saveDialog != nullptr)
        {
            const auto name = saveDialog->getTextEditorContents ("name");
            if (proc.saveUserPreset (name))
                refreshPresetBox();
        }
}

// ============================================================================
GobblerEditor::GobblerEditor (GobblerProcessor& p) : AudioProcessorEditor (&p), content (p)
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (content);
    content.setBounds (0, 0, baseW, baseH);

    setResizable (true, true);
    setResizeLimits (700, 490, 1800, 1260);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) baseW / (double) baseH);
    setSize (baseW, baseH);
}

GobblerEditor::~GobblerEditor() { setLookAndFeel (nullptr); }

void GobblerEditor::paint (Graphics& g) { g.fillAll (Colours_::bg); }

void GobblerEditor::resized()
{
    // scale the whole fixed-size layout to the window
    content.setTransform (AffineTransform::scale ((float) getWidth() / (float) baseW));
}
