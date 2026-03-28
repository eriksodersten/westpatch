#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <WestPatchBinaryData.h>

static const juce::Colour kAccent    { 0xffc8a040 };
static const juce::Colour kSea       { 0xff4ab0bc };
static const juce::Colour kPanel     { 0x48000000 };
static const juce::Colour kBorder    { 0x1affffff };
static const juce::Colour kLabelCol  { 0x88ffffff };
static const juce::Colour kScrew     { 0x33ffffff };

WestPatchAudioProcessorEditor::WestPatchAudioProcessorEditor (WestPatchAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1340, 820);

    backgroundImage = juce::ImageFileFormat::loadFrom (
        WestPatchBinaryData::background_jpg,
        WestPatchBinaryData::background_jpgSize);

    // Top bar – group mode
    groupModeBox.addItem ("Unison", 1);
    groupModeBox.addItem ("Duo",    2);
    groupModeBox.addItem ("Quad",   3);
    switch (audioProcessor.groupMode)
    {
        case GroupMode::Mono: groupModeBox.setSelectedId (1, juce::dontSendNotification); break;
        case GroupMode::Duo:  groupModeBox.setSelectedId (2, juce::dontSendNotification); break;
        case GroupMode::Quad: groupModeBox.setSelectedId (3, juce::dontSendNotification); break;
        default:              groupModeBox.setSelectedId (1, juce::dontSendNotification); break;
    }
    groupModeBox.onChange = [this]
    {
        switch (groupModeBox.getSelectedId())
        {
            case 1: audioProcessor.setGroupMode (GroupMode::Mono); break;
            case 2: audioProcessor.setGroupMode (GroupMode::Duo);  break;
            case 3: audioProcessor.setGroupMode (GroupMode::Quad); break;
            default: break;
        }
    };
    addAndMakeVisible (groupModeBox);

    // Voice
    setupLabel (attackLabel,  "Attack");  addAndMakeVisible (attackLabel);
    setupKnob  (attackSlider, 0.001, 5.0, 0.001);
    attackSlider.setValue (audioProcessor.attackTime, juce::dontSendNotification);
    attackSlider.onValueChange = [this] { audioProcessor.attackTime = (float) attackSlider.getValue(); };
    addAndMakeVisible (attackSlider);

    setupLabel (releaseLabel,  "Release"); addAndMakeVisible (releaseLabel);
    setupKnob  (releaseSlider, 0.001, 5.0, 0.001);
    releaseSlider.setValue (audioProcessor.releaseTime, juce::dontSendNotification);
    releaseSlider.onValueChange = [this] { audioProcessor.releaseTime = (float) releaseSlider.getValue(); };
    addAndMakeVisible (releaseSlider);

    setupLabel (foldLabel,  "Fold"); addAndMakeVisible (foldLabel);
    setupKnob  (foldSlider, 0.0, 5.0, 0.01);
    foldSlider.setValue (audioProcessor.foldAmount - 0.05, juce::dontSendNotification);
    foldSlider.onValueChange = [this] { audioProcessor.foldAmount = (float) foldSlider.getValue() + 0.05f; };
    addAndMakeVisible (foldSlider);

    setupLabel (lpgLabel,  "LPG"); addAndMakeVisible (lpgLabel);
    setupKnob  (lpgSlider, 0.0, 1.0, 0.001);
    lpgSlider.setValue (audioProcessor.lpgAmount, juce::dontSendNotification);
    lpgSlider.onValueChange = [this] { audioProcessor.lpgAmount = (float) lpgSlider.getValue(); };
    addAndMakeVisible (lpgSlider);

    // 281
    setupLabel (mod281AttackLabel, "Attack"); addAndMakeVisible (mod281AttackLabel);
    setupKnob  (mod281AttackSlider, 0.0, 1.0, 0.001);
        mod281AttackSlider.setValue (std::log10 (audioProcessor.modAttackTime / 0.001) / 4.0, juce::dontSendNotification);
        mod281AttackSlider.onValueChange = [this]
        {
            audioProcessor.modAttackTime = 0.001f * std::pow (10.0f, (float) mod281AttackSlider.getValue() * 4.0f);
        };
    addAndMakeVisible (mod281AttackSlider);

    setupLabel (mod281DecayLabel, "Decay"); addAndMakeVisible (mod281DecayLabel);
    setupKnob  (mod281DecaySlider, 0.0, 1.0, 0.001);
        mod281DecaySlider.setValue (std::log10 (audioProcessor.modReleaseTime / 0.001) / 4.0, juce::dontSendNotification);
        mod281DecaySlider.onValueChange = [this]
        {
            audioProcessor.modReleaseTime = 0.001f * std::pow (10.0f, (float) mod281DecaySlider.getValue() * 4.0f);
        };
    addAndMakeVisible (mod281DecaySlider);

    func281ModeBox.addItem ("Transient", 1);
    func281ModeBox.addItem ("Sustain",   2);
    func281ModeBox.addItem ("Cycle",     3);
    switch (audioProcessor.func281Mode)
    {
        case FunctionGenerator281::Mode::Transient: func281ModeBox.setSelectedId (1, juce::dontSendNotification); break;
        case FunctionGenerator281::Mode::Sustain:   func281ModeBox.setSelectedId (2, juce::dontSendNotification); break;
        case FunctionGenerator281::Mode::Cycle:     func281ModeBox.setSelectedId (3, juce::dontSendNotification); break;
        default:                                    func281ModeBox.setSelectedId (1, juce::dontSendNotification); break;
    }
    func281ModeBox.onChange = [this]
    {
        switch (func281ModeBox.getSelectedId())
        {
            case 1: audioProcessor.func281Mode = FunctionGenerator281::Mode::Transient; break;
            case 2: audioProcessor.func281Mode = FunctionGenerator281::Mode::Sustain;   break;
            case 3: audioProcessor.func281Mode = FunctionGenerator281::Mode::Cycle;     break;
            default: break;
        }
    };
    addAndMakeVisible (func281ModeBox);

    setupLabel (mod281PitchDepthLabel, "Pitch"); addAndMakeVisible (mod281PitchDepthLabel);
    setupDepthSlider (mod281PitchDepthSlider);
    mod281PitchDepthSlider.setValue (audioProcessor.modulationMatrix[(int)ModSource::FunctionB][(int)ModDestination::Pitch], juce::dontSendNotification);
    mod281PitchDepthSlider.onValueChange = [this] { audioProcessor.modulationMatrix[(int)ModSource::FunctionB][(int)ModDestination::Pitch] = (float)mod281PitchDepthSlider.getValue(); };
    addAndMakeVisible (mod281PitchDepthSlider);

    setupLabel (mod281FoldDepthLabel, "Fold"); addAndMakeVisible (mod281FoldDepthLabel);
    setupDepthSlider (mod281FoldDepthSlider);
    mod281FoldDepthSlider.setValue (audioProcessor.modulationMatrix[(int)ModSource::FunctionB][(int)ModDestination::Fold], juce::dontSendNotification);
    mod281FoldDepthSlider.onValueChange = [this] { audioProcessor.modulationMatrix[(int)ModSource::FunctionB][(int)ModDestination::Fold] = (float)mod281FoldDepthSlider.getValue(); };
    addAndMakeVisible (mod281FoldDepthSlider);

    setupLabel (mod281LpgDepthLabel, "LPG"); addAndMakeVisible (mod281LpgDepthLabel);
    setupDepthSlider (mod281LpgDepthSlider);
    mod281LpgDepthSlider.setValue (audioProcessor.modulationMatrix[(int)ModSource::FunctionB][(int)ModDestination::LPG], juce::dontSendNotification);
    mod281LpgDepthSlider.onValueChange = [this] { audioProcessor.modulationMatrix[(int)ModSource::FunctionB][(int)ModDestination::LPG] = (float)mod281LpgDepthSlider.getValue(); };
    addAndMakeVisible (mod281LpgDepthSlider);

    // 266
    setupLabel (uncertaintyRateLabel, "Rate"); addAndMakeVisible (uncertaintyRateLabel);
    setupKnob  (uncertaintyRateSlider, 0.0, 20.0, 0.01);
    uncertaintyRateSlider.setValue (audioProcessor.uncertaintyRate, juce::dontSendNotification);
    uncertaintyRateSlider.onValueChange = [this] { audioProcessor.uncertaintyRate = (float) uncertaintyRateSlider.getValue(); };
    addAndMakeVisible (uncertaintyRateSlider);

    setupLabel (uncertaintySmoothLabel, "Smooth"); addAndMakeVisible (uncertaintySmoothLabel);
    setupKnob  (uncertaintySmoothSlider, 0.0, 1.0, 0.001);
    uncertaintySmoothSlider.setValue (audioProcessor.uncertaintySmoothDepth, juce::dontSendNotification);
    uncertaintySmoothSlider.onValueChange = [this] { audioProcessor.uncertaintySmoothDepth = (float) uncertaintySmoothSlider.getValue(); };
    addAndMakeVisible (uncertaintySmoothSlider);

    setupLabel (uncertaintySteppedLabel, "Stepped"); addAndMakeVisible (uncertaintySteppedLabel);
    setupKnob  (uncertaintySteppedSlider, 0.0, 24.0, 0.01);
    uncertaintySteppedSlider.setValue (audioProcessor.uncertaintySteppedDepth, juce::dontSendNotification);
    uncertaintySteppedSlider.onValueChange = [this] { audioProcessor.uncertaintySteppedDepth = (float) uncertaintySteppedSlider.getValue(); };
    addAndMakeVisible (uncertaintySteppedSlider);

    // Mixer
    setupLabel (synthLevelLabel, "Synth"); addAndMakeVisible (synthLevelLabel);
    setupKnob  (synthLevelSlider, 0.0, 1.0, 0.001);
    synthLevelSlider.setValue (audioProcessor.synthLevel, juce::dontSendNotification);
    synthLevelSlider.onValueChange = [this] { audioProcessor.synthLevel = (float) synthLevelSlider.getValue(); };
    addAndMakeVisible (synthLevelSlider);

    setupLabel (inputLevelLabel, "Input"); addAndMakeVisible (inputLevelLabel);
    setupKnob  (inputLevelSlider, 0.0, 1.0, 0.001);
    inputLevelSlider.setValue (audioProcessor.inputLevel, juce::dontSendNotification);
    inputLevelSlider.onValueChange = [this] { audioProcessor.inputLevel = (float) inputLevelSlider.getValue(); };
    addAndMakeVisible (inputLevelSlider);

    setupLabel (noiseLevelLabel, "Noise"); addAndMakeVisible (noiseLevelLabel);
    setupKnob  (noiseLevelSlider, 0.0, 1.0, 0.001);
    noiseLevelSlider.setValue (audioProcessor.noiseLevel, juce::dontSendNotification);
    noiseLevelSlider.onValueChange = [this] { audioProcessor.noiseLevel = (float) noiseLevelSlider.getValue(); };
    addAndMakeVisible (noiseLevelSlider);

    gateInputButton.setButtonText ("Gate");
    gateInputButton.setToggleState (audioProcessor.gateExternalInput, juce::dontSendNotification);
    gateInputButton.onClick = [this] { audioProcessor.gateExternalInput = gateInputButton.getToggleState(); };
    addAndMakeVisible (gateInputButton);

    // Lane / complex
    setupLabel (detuneAmountLabel, "Detune"); addAndMakeVisible (detuneAmountLabel);
    setupKnob  (detuneAmountSlider, 0.0, 1.0, 0.001);
    detuneAmountSlider.setValue (audioProcessor.detuneAmount, juce::dontSendNotification);
    detuneAmountSlider.onValueChange = [this] { audioProcessor.detuneAmount = (float) detuneAmountSlider.getValue(); };
    addAndMakeVisible (detuneAmountSlider);

    setupLabel (stereoSpreadLabel, "Spread"); addAndMakeVisible (stereoSpreadLabel);
    setupKnob  (stereoSpreadSlider, 0.0, 1.0, 0.001);
    stereoSpreadSlider.setValue (audioProcessor.stereoSpread, juce::dontSendNotification);
    stereoSpreadSlider.onValueChange = [this] { audioProcessor.stereoSpread = (float) stereoSpreadSlider.getValue(); };
    addAndMakeVisible (stereoSpreadSlider);

    setupLabel (complexModRatioLabel, "Ratio"); addAndMakeVisible (complexModRatioLabel);
    setupKnob  (complexModRatioSlider, 0.1, 8.0, 0.01);
    complexModRatioSlider.setValue (audioProcessor.complexModRatio, juce::dontSendNotification);
    complexModRatioSlider.onValueChange = [this] { audioProcessor.complexModRatio = (float) complexModRatioSlider.getValue(); };
    addAndMakeVisible (complexModRatioSlider);

    setupLabel (complexFmAmountLabel, "FM"); addAndMakeVisible (complexFmAmountLabel);
    setupKnob  (complexFmAmountSlider, 0.0, 100.0, 0.01);
    complexFmAmountSlider.setValue (audioProcessor.complexFmAmount, juce::dontSendNotification);
    complexFmAmountSlider.onValueChange = [this] { audioProcessor.complexFmAmount = (float) complexFmAmountSlider.getValue(); };
    addAndMakeVisible (complexFmAmountSlider);

    setupLabel (complexOscMixLabel, "Mix"); addAndMakeVisible (complexOscMixLabel);
    setupKnob  (complexOscMixSlider, 0.0, 1.0, 0.001);
    complexOscMixSlider.setValue (audioProcessor.complexOscMix, juce::dontSendNotification);
    complexOscMixSlider.onValueChange = [this] { audioProcessor.complexOscMix = (float) complexOscMixSlider.getValue(); };
    addAndMakeVisible (complexOscMixSlider);

    // Matrix
    auto setupMx = [this] (juce::Slider& s, int src, int dst)
    {
        setupMatrixSlider (s);
        s.setValue (audioProcessor.modulationMatrix[src][dst], juce::dontSendNotification);
        s.onValueChange = [this, &s, src, dst] { audioProcessor.modulationMatrix[src][dst] = (float) s.getValue(); };
        addAndMakeVisible (s);
    };

    const int srcB  = (int) ModSource::FunctionB;
        const int srcUS = (int) ModSource::UncertaintySmooth;
        const int srcUT = (int) ModSource::UncertaintyStepped;
        const int dstP  = (int) ModDestination::Pitch;
        const int dstF  = (int) ModDestination::Fold;
        const int dstL  = (int) ModDestination::LPG;

    setupMx (matrix281PitchSlider,         srcB,  dstP);
        setupMx (matrix281FoldSlider,          srcB,  dstF);
        setupMx (matrix281LpgSlider,           srcB,  dstL);
        setupMx (matrix266SmoothPitchSlider,   srcUS, dstP);
        setupMx (matrix266SmoothFoldSlider,    srcUS, dstF);
        setupMx (matrix266SmoothLpgSlider,     srcUS, dstL);
        setupMx (matrix266SteppedPitchSlider,  srcUT, dstP);
        setupMx (matrix266SteppedFoldSlider,   srcUT, dstF);
        setupMx (matrix266SteppedLpgSlider,    srcUT, dstL);
}

WestPatchAudioProcessorEditor::~WestPatchAudioProcessorEditor() = default;

void WestPatchAudioProcessorEditor::setupKnob (juce::Slider& s, double mn, double mx, double interval)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
    s.setRange (mn, mx, interval);
    s.setColour (juce::Slider::rotarySliderFillColourId,    kAccent);
    s.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::white.withAlpha (0.15f));
    s.setColour (juce::Slider::thumbColourId,               kAccent);
    s.setColour (juce::Slider::textBoxTextColourId,         juce::Colours::white.withAlpha (0.45f));
    s.setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    s.setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
}

void WestPatchAudioProcessorEditor::setupDepthSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::LinearHorizontal);
    s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 40, 16);
    s.setRange (-1.0, 1.0, 0.001);
    s.setDoubleClickReturnValue (true, 0.0);
    s.setColour (juce::Slider::trackColourId,           kAccent.withAlpha (0.6f));
    s.setColour (juce::Slider::thumbColourId,           kAccent);
    s.setColour (juce::Slider::backgroundColourId,      juce::Colours::white.withAlpha (0.08f));
    s.setColour (juce::Slider::textBoxTextColourId,     juce::Colours::white.withAlpha (0.45f));
    s.setColour (juce::Slider::textBoxOutlineColourId,  juce::Colours::transparentBlack);
    s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}

void WestPatchAudioProcessorEditor::setupMatrixSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::LinearVertical);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 44, 14);
    s.setRange (-1.0, 1.0, 0.001);
    s.setDoubleClickReturnValue (true, 0.0);
    s.setColour (juce::Slider::trackColourId,           kAccent.withAlpha (0.6f));
    s.setColour (juce::Slider::thumbColourId,           kAccent);
    s.setColour (juce::Slider::backgroundColourId,      juce::Colours::white.withAlpha (0.08f));
    s.setColour (juce::Slider::textBoxTextColourId,     juce::Colours::white.withAlpha (0.4f));
    s.setColour (juce::Slider::textBoxOutlineColourId,  juce::Colours::transparentBlack);
    s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
}

void WestPatchAudioProcessorEditor::setupLabel (juce::Label& label,
                                                const juce::String& text,
                                                juce::Justification j)
{
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (j);
    label.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.5f));
    label.setFont (juce::FontOptions (10.0f));
}

void WestPatchAudioProcessorEditor::drawModule (juce::Graphics& g,
                                                juce::Rectangle<int> b,
                                                const juce::String& title) const
{
    g.setColour (kPanel);
    g.fillRoundedRectangle (b.toFloat(), 3.0f);
    g.setColour (kBorder);
    g.drawRoundedRectangle (b.toFloat(), 3.0f, 0.5f);

    // screws
    const int sr = 5;
    const auto corners = std::array<juce::Point<int>, 4> {{
        { b.getX() + 10,         b.getY() + 10 },
        { b.getRight() - 10,     b.getY() + 10 },
        { b.getX() + 10,         b.getBottom() - 10 },
        { b.getRight() - 10,     b.getBottom() - 10 }
    }};
    for (auto& c : corners)
    {
        g.setColour (kScrew);
        g.fillEllipse (c.x - sr, c.y - sr, sr * 2, sr * 2);
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.drawEllipse (c.x - sr, c.y - sr, sr * 2, sr * 2, 0.5f);
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawLine (c.x - 2, c.y, c.x + 2, c.y, 0.5f);
    }

    // title
    g.setColour (juce::Colours::white.withAlpha (0.32f));
    g.setFont (juce::FontOptions (9.0f));
    g.drawFittedText (title.toUpperCase(),
                      b.getX() + 18, b.getY() + 18,
                      b.getWidth() - 36, 14,
                      juce::Justification::centred, 1);
}

void WestPatchAudioProcessorEditor::paint (juce::Graphics& g)
{
    if (backgroundImage.isValid())
        g.drawImage (backgroundImage, getLocalBounds().toFloat(),
                     juce::RectanglePlacement::fillDestination);
    else
        g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillRect (getLocalBounds());

    // module backgrounds
    drawModule (g, { 20,  38, 168, 760 }, "Voice");
    drawModule (g, { 196, 38, 320, 760 }, "281");
    drawModule (g, { 524, 38, 150, 760 }, "266");
    drawModule (g, { 682, 38, 148, 760 }, "Mixer");
    drawModule (g, { 838, 38, 234, 760 }, "Lane");
    drawModule (g, { 1080,38, 240, 760 }, "Matrix");

    // logo
    g.setColour (juce::Colours::white.withAlpha (0.55f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawFittedText ("WESTPATCH", 20, 8, 300, 22,
                      juce::Justification::centredLeft, 1);

    // matrix column guides
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    const int mx0 = 1080 + 20 + 52;
    const int colW = 56;
    for (int i = 0; i < 3; ++i)
        g.drawVerticalLine (mx0 + i * colW + 28, 38 + 60, 38 + 720);
}

void WestPatchAudioProcessorEditor::resized()
{
    // Top bar – group mode
    groupModeBox.setBounds (250, 8, 100, 22);

    const int knobW = 64;
    const int knobH = 86;

    // ── Voice module ── x=20 w=168
    {
        const int mx = 20;
        const int cx = mx + 84 - knobW / 2;
        int y = 60;
        const int gap = 92;

        attackLabel.setBounds  (cx, y,        knobW, 14); y += 14;
        attackSlider.setBounds (cx, y,        knobW, knobH); y += knobH + gap - 14;

        releaseLabel.setBounds  (cx, y,       knobW, 14); y += 14;
        releaseSlider.setBounds (cx, y,       knobW, knobH); y += knobH + gap - 14;

        foldLabel.setBounds  (cx, y,          knobW, 14); y += 14;
        foldSlider.setBounds (cx, y,          knobW, knobH); y += knobH + gap - 14;

        lpgLabel.setBounds  (cx, y,           knobW, 14); y += 14;
        lpgSlider.setBounds (cx, y,           knobW, knobH);
    }

    // ── 281 module ── x=196 w=320
    {
        const int mx = 196;
        const int bigKnob = 80;
        int y = 56;

        // Two big knobs side by side
        mod281AttackLabel.setBounds  (mx + 24,        y, bigKnob, 14);
        mod281DecayLabel.setBounds   (mx + 24 + 100,  y, bigKnob, 14);
        y += 14;
        mod281AttackSlider.setBounds (mx + 14,        y, bigKnob, bigKnob + 20);
        mod281DecaySlider.setBounds  (mx + 14 + 100,  y, bigKnob, bigKnob + 20);
        y += bigKnob + 28;

        // Mode selector
        func281ModeBox.setBounds (mx + 14, y, 180, 24);
        y += 36;

        // Depth sliders
        const int dW = 280;
        const int dH = 28;
        const int dX = mx + 14;
        const int labelW = 36;

        mod281PitchDepthLabel.setBounds (dX, y, labelW, dH);
        mod281PitchDepthSlider.setBounds (dX + labelW, y, dW - labelW, dH);
        y += dH + 8;

        mod281FoldDepthLabel.setBounds (dX, y, labelW, dH);
        mod281FoldDepthSlider.setBounds (dX + labelW, y, dW - labelW, dH);
        y += dH + 8;

        mod281LpgDepthLabel.setBounds (dX, y, labelW, dH);
        mod281LpgDepthSlider.setBounds (dX + labelW, y, dW - labelW, dH);
    }

    // ── 266 module ── x=524 w=150
    {
        const int mx = 524;
        const int cx = mx + 75 - knobW / 2;
        int y = 60;
        const int gap = 92;

        uncertaintyRateLabel.setBounds   (cx, y, knobW, 14); y += 14;
        uncertaintyRateSlider.setBounds  (cx, y, knobW, knobH); y += knobH + gap - 14;

        uncertaintySmoothLabel.setBounds  (cx, y, knobW, 14); y += 14;
        uncertaintySmoothSlider.setBounds (cx, y, knobW, knobH); y += knobH + gap - 14;

        uncertaintySteppedLabel.setBounds  (cx, y, knobW, 14); y += 14;
        uncertaintySteppedSlider.setBounds (cx, y, knobW, knobH);
    }

    // ── Mixer module ── x=682 w=148
    {
        const int mx = 682;
        const int cx = mx + 74 - knobW / 2;
        int y = 60;
        const int gap = 92;

        synthLevelLabel.setBounds  (cx, y, knobW, 14); y += 14;
        synthLevelSlider.setBounds (cx, y, knobW, knobH); y += knobH + gap - 14;

        inputLevelLabel.setBounds  (cx, y, knobW, 14); y += 14;
        inputLevelSlider.setBounds (cx, y, knobW, knobH); y += knobH + gap - 14;

        noiseLevelLabel.setBounds  (cx, y, knobW, 14); y += 14;
        noiseLevelSlider.setBounds (cx, y, knobW, knobH); y += knobH + 16;

        gateInputButton.setBounds (mx + 14, y, 120, 22);
    }

    // ── Lane module ── x=838 w=234
    {
        const int mx = 838;
        const int cx = mx + 117 - knobW / 2;
        int y = 60;
        const int gap = 92;

        detuneAmountLabel.setBounds   (cx, y, knobW, 14); y += 14;
        detuneAmountSlider.setBounds  (cx, y, knobW, knobH); y += knobH + gap - 14;

        stereoSpreadLabel.setBounds   (cx, y, knobW, 14); y += 14;
        stereoSpreadSlider.setBounds  (cx, y, knobW, knobH); y += knobH + gap - 14;

        complexModRatioLabel.setBounds (cx, y, knobW, 14); y += 14;
        complexModRatioSlider.setBounds(cx, y, knobW, knobH); y += knobH + gap - 14;

        complexFmAmountLabel.setBounds (cx, y, knobW, 14); y += 14;
        complexFmAmountSlider.setBounds(cx, y, knobW, knobH); y += knobH + gap - 14;

        complexOscMixLabel.setBounds   (cx, y, knobW, 14); y += 14;
        complexOscMixSlider.setBounds  (cx, y, knobW, knobH);
    }

    // ── Matrix module ── x=1080 w=240
    {
        const int mx = 1080;
        const int innerX = mx + 20;
        const int srcW = 52;
        const int slW = 44;
        const int slH = 80;
        const int colW = 56;
        const int rowGap = 88;
        int y = 58;

        matrix281PitchSlider.setBounds        (innerX + srcW + 0 * colW, y, slW, slH);
        matrix281FoldSlider.setBounds         (innerX + srcW + 1 * colW, y, slW, slH);
        matrix281LpgSlider.setBounds          (innerX + srcW + 2 * colW, y, slW, slH);
        y += rowGap;

        matrix266SmoothPitchSlider.setBounds  (innerX + srcW + 0 * colW, y, slW, slH);
        matrix266SmoothFoldSlider.setBounds   (innerX + srcW + 1 * colW, y, slW, slH);
        matrix266SmoothLpgSlider.setBounds    (innerX + srcW + 2 * colW, y, slW, slH);
        y += rowGap;

        matrix266SteppedPitchSlider.setBounds (innerX + srcW + 0 * colW, y, slW, slH);
        matrix266SteppedFoldSlider.setBounds  (innerX + srcW + 1 * colW, y, slW, slH);
        matrix266SteppedLpgSlider.setBounds   (innerX + srcW + 2 * colW, y, slW, slH);
    }
}
