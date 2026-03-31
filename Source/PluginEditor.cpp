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
    setSize (1500, 820);

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

        lpgModeBox.addItem ("Lopass", 1);
        lpgModeBox.addItem ("Gate",   2);
        lpgModeBox.addItem ("Combo",  3);
        switch (audioProcessor.lpgMode)
        {
            case LowPassGate::Mode::Lopass: lpgModeBox.setSelectedId (1, juce::dontSendNotification); break;
            case LowPassGate::Mode::Gate:   lpgModeBox.setSelectedId (2, juce::dontSendNotification); break;
            case LowPassGate::Mode::Combo:  lpgModeBox.setSelectedId (3, juce::dontSendNotification); break;
            default:                        lpgModeBox.setSelectedId (3, juce::dontSendNotification); break;
        }
        lpgModeBox.onChange = [this]
        {
            switch (lpgModeBox.getSelectedId())
            {
                case 1: audioProcessor.lpgMode = LowPassGate::Mode::Lopass; break;
                case 2: audioProcessor.lpgMode = LowPassGate::Mode::Gate;   break;
                case 3: audioProcessor.lpgMode = LowPassGate::Mode::Combo;  break;
                default: break;
            }
        };
    addAndMakeVisible (lpgModeBox);

        setupLabel (glideLabel, "Glide"); addAndMakeVisible (glideLabel);
        setupKnob  (glideSlider, 0.0, 2.0, 0.001);
        glideSlider.setValue (audioProcessor.glideTime, juce::dontSendNotification);
    glideSlider.onValueChange = [this]
        {
            if (glideToggle.getToggleState())
                audioProcessor.setGlideTime ((float) glideSlider.getValue());
        };
        addAndMakeVisible (glideSlider);

        glideToggle.setToggleState (audioProcessor.glideTime > 0.0f, juce::dontSendNotification);
    glideToggle.onStateChange = [this]
        {
            if (glideToggle.getToggleState())
                audioProcessor.setGlideTime ((float) glideSlider.getValue());
            else
                audioProcessor.setGlideTime (0.0f);
        };
        addAndMakeVisible (glideToggle);

    // 281
    setupLabel (mod281AttackLabel, "Attack"); addAndMakeVisible (mod281AttackLabel);
    setupKnob  (mod281AttackSlider, 0.0, 1.0, 0.001);
        mod281AttackSlider.setValue (std::log10 (audioProcessor.modAttackTime / 0.001) / 4.0, juce::dontSendNotification);
    mod281AttackSlider.onValueChange = [this]
            {
                if (audioProcessor.func281SyncEnabled)
                    audioProcessor.func281SyncAttackIndex = juce::roundToInt (mod281AttackSlider.getValue() * 12.0);
                else
                    audioProcessor.modAttackTime = 0.001f * std::pow (10.0f, (float) mod281AttackSlider.getValue() * 4.0f);
            };
    addAndMakeVisible (mod281AttackSlider);

    setupLabel (mod281DecayLabel, "Decay"); addAndMakeVisible (mod281DecayLabel);
    setupKnob  (mod281DecaySlider, 0.0, 1.0, 0.001);
        mod281DecaySlider.setValue (std::log10 (audioProcessor.modReleaseTime / 0.001) / 4.0, juce::dontSendNotification);
    mod281DecaySlider.onValueChange = [this]
            {
                if (audioProcessor.func281SyncEnabled)
                    audioProcessor.func281SyncDecayIndex = juce::roundToInt (mod281DecaySlider.getValue() * 12.0);
                else
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

    func281SyncButton.setClickingTogglesState (true);
        func281SyncButton.setToggleState (audioProcessor.func281SyncEnabled, juce::dontSendNotification);
        func281SyncButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::orange);
        func281SyncButton.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        func281SyncButton.onClick = [this]
        {
            audioProcessor.func281SyncEnabled = func281SyncButton.getToggleState();
            updateSyncMode();
        };
        addAndMakeVisible (func281SyncButton);


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

        setupLabel (uncertaintySlewLabel, "Slew"); addAndMakeVisible (uncertaintySlewLabel);
        setupKnob  (uncertaintySlewSlider, 0.0, 2.0, 0.001);
        uncertaintySlewSlider.setValue (audioProcessor.uncertaintySlewTime, juce::dontSendNotification);
        uncertaintySlewSlider.onValueChange = [this] { audioProcessor.uncertaintySlewTime = (float) uncertaintySlewSlider.getValue(); };
        addAndMakeVisible (uncertaintySlewSlider);

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

        setupLabel (oscShapeLabel, "Shape"); addAndMakeVisible (oscShapeLabel);
        setupKnob  (oscShapeSlider, 0.0, 1.0, 0.001);
        oscShapeSlider.setValue (audioProcessor.oscShape, juce::dontSendNotification);
        oscShapeSlider.onValueChange = [this] { audioProcessor.oscShape = (float) oscShapeSlider.getValue(); };
        addAndMakeVisible (oscShapeSlider);

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

    const int dstA  = (int) ModDestination::Mod281Attack;
            const int dstD  = (int) ModDestination::Mod281Decay;

            setupMx (matrix281PitchSlider,              srcB,  dstP);
            setupMx (matrix281FoldSlider,               srcB,  dstF);
            setupMx (matrix281LpgSlider,                srcB,  dstL);
            setupMx (matrix281Mod281AttackSlider,        srcB,  dstA);
            setupMx (matrix281Mod281DecaySlider,         srcB,  dstD);
            setupMx (matrix266SmoothPitchSlider,        srcUS, dstP);
            setupMx (matrix266SmoothFoldSlider,         srcUS, dstF);
            setupMx (matrix266SmoothLpgSlider,          srcUS, dstL);
            setupMx (matrix266SmoothMod281AttackSlider, srcUS, dstA);
            setupMx (matrix266SmoothMod281DecaySlider,  srcUS, dstD);
            setupMx (matrix266SteppedPitchSlider,       srcUT, dstP);
            setupMx (matrix266SteppedFoldSlider,        srcUT, dstF);
            setupMx (matrix266SteppedLpgSlider,         srcUT, dstL);
            setupMx (matrix266SteppedMod281AttackSlider,srcUT, dstA);
            setupMx (matrix266SteppedMod281DecaySlider, srcUT, dstD);

        // Self-mod knob i 281-modulen
        setupLabel (func281SelfModLabel, "Self"); addAndMakeVisible (func281SelfModLabel);
        setupKnob  (func281SelfModSlider, -1.0, 2.0, 0.01);
        func281SelfModSlider.setValue (audioProcessor.func281SelfModAmount, juce::dontSendNotification);
        func281SelfModSlider.onValueChange = [this]
        {
            audioProcessor.func281SelfModAmount = (float) func281SelfModSlider.getValue();
        };
        addAndMakeVisible (func281SelfModSlider);
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
    drawModule (g, { 1060,38, 420, 760 }, "Matrix");

    // logo
    g.setColour (juce::Colours::white.withAlpha (0.55f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawFittedText ("WESTPATCH", 20, 8, 300, 22,
                      juce::Justification::centredLeft, 1);

    // matrix legend
        {
            const int mx0    = 1060 + 20;
            const int srcW   = 52;
            const int colW   = 48;
            const int rowGap = 88;
            const int slH    = 80;
            const int slW    = 38;
            const int startY = 38 + 74;

            g.setFont (juce::FontOptions (9.0f));

            // Kolumnlabels
            const char* colLabels[] = { "Pitch", "Fold", "LPG", "A-time", "D-time" };
                    g.setColour (juce::Colours::white.withAlpha (0.4f));
                    for (int i = 0; i < 5; ++i)
                        g.drawFittedText (colLabels[i],
                                                      mx0 + srcW + i * colW - 4, startY - 38,
                                                      colW + 8, 14,
                                                      juce::Justification::centred, 1);

            // Radlabels
            const char* rowLabels[] = { "281", "Smooth", "Stepped" };
            for (int r = 0; r < 3; ++r)
                g.drawFittedText (rowLabels[r],
                                  mx0, startY + r * rowGap + slH / 2 - 6,
                                  srcW - 4, 12,
                                  juce::Justification::centredRight, 1);

            // Vertikala separatorlinjer
            g.setColour (juce::Colours::white.withAlpha (0.08f));
            for (int i = 0; i < 5; ++i)
                g.drawVerticalLine (mx0 + srcW + i * colW + slW, startY, startY + 3 * rowGap);
        }
}

void WestPatchAudioProcessorEditor::updateSyncMode()
{
    const bool sync = audioProcessor.func281SyncEnabled;
    if (sync)
    {
        static const char* ratioNames[] =
                    { "1/8","1/7","1/5","1/4","1/3","1/2","1","2","3","4","5","7","8" };

                mod281AttackSlider.setRange (0.0, 12.0, 1.0);
                mod281AttackSlider.setValue (audioProcessor.func281SyncAttackIndex, juce::dontSendNotification);
                mod281AttackSlider.textFromValueFunction = [](double v)
                    { return juce::String (ratioNames[juce::jlimit (0, 12, (int) v)]); };

                mod281DecaySlider.setRange (0.0, 12.0, 1.0);
                mod281DecaySlider.setValue (audioProcessor.func281SyncDecayIndex, juce::dontSendNotification);
                mod281DecaySlider.textFromValueFunction = [](double v)
                    { return juce::String (ratioNames[juce::jlimit (0, 12, (int) v)]); };
    }
    else
    {
        mod281AttackSlider.textFromValueFunction = nullptr;
                mod281AttackSlider.setRange (0.0, 1.0, 0.001);
                mod281AttackSlider.setValue (std::log10 (audioProcessor.modAttackTime / 0.001) / 4.0, juce::dontSendNotification);
                mod281DecaySlider.textFromValueFunction = nullptr;
                mod281DecaySlider.setRange (0.0, 1.0, 0.001);
                mod281DecaySlider.setValue (std::log10 (audioProcessor.modReleaseTime / 0.001) / 4.0, juce::dontSendNotification);
    }
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

        lpgLabel.setBounds    (cx, y, knobW, 14); y += 14;
                lpgSlider.setBounds   (cx, y, knobW, knobH);
                lpgModeBox.setBounds  (cx + knobW + 4, y + (knobH / 2) - 10, 96, 20);
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
                y += 32;

        func281SyncButton.setBounds (mx + 14, y, 52, 24);
                                y += 32;

                        func281SelfModLabel.setBounds  (mx + 14, y, 44, 14);
                        func281SelfModSlider.setBounds (mx + 14, y + 14, 64, 64);
                        y += 82;

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
                y += dH + 16;

                glideToggle.setBounds (dX, y, 60, 20);
                glideLabel.setBounds  (dX + 68, y, knobW, 14);
                glideSlider.setBounds (dX + 68, y + 14, knobW, knobH);
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
                uncertaintySteppedSlider.setBounds (cx, y, knobW, knobH); y += knobH + gap - 14;

                uncertaintySlewLabel.setBounds  (cx, y, knobW, 14); y += 14;
                uncertaintySlewSlider.setBounds (cx, y, knobW, knobH);
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
            int y = 56;
            const int laneKnobH = 72;
            const int laneGap   = 10;

            detuneAmountLabel.setBounds   (cx, y, knobW, 14); y += 14;
            detuneAmountSlider.setBounds  (cx, y, knobW, laneKnobH); y += laneKnobH + laneGap;

            stereoSpreadLabel.setBounds   (cx, y, knobW, 14); y += 14;
            stereoSpreadSlider.setBounds  (cx, y, knobW, laneKnobH); y += laneKnobH + laneGap;

            complexModRatioLabel.setBounds (cx, y, knobW, 14); y += 14;
            complexModRatioSlider.setBounds(cx, y, knobW, laneKnobH); y += laneKnobH + laneGap;

            complexFmAmountLabel.setBounds (cx, y, knobW, 14); y += 14;
            complexFmAmountSlider.setBounds(cx, y, knobW, laneKnobH); y += laneKnobH + laneGap;

            complexOscMixLabel.setBounds   (cx, y, knobW, 14); y += 14;
            complexOscMixSlider.setBounds  (cx, y, knobW, laneKnobH); y += laneKnobH + laneGap;

            oscShapeLabel.setBounds  (cx, y, knobW, 14); y += 14;
            oscShapeSlider.setBounds (cx, y, knobW, laneKnobH);
        }

    // ── Matrix module ── x=1080 w=240
    {
        const int mx = 1060;
                const int innerX = mx + 20;
                const int srcW = 52;
                const int slW = 38;
                const int slH = 80;
                const int colW = 48;
        const int rowGap = 88;
                int y = 74;

        matrix281PitchSlider.setBounds              (innerX + srcW + 0 * colW, y, slW, slH);
                matrix281FoldSlider.setBounds               (innerX + srcW + 1 * colW, y, slW, slH);
                matrix281LpgSlider.setBounds                (innerX + srcW + 2 * colW, y, slW, slH);
                matrix281Mod281AttackSlider.setBounds        (innerX + srcW + 3 * colW, y, slW, slH);
                matrix281Mod281DecaySlider.setBounds         (innerX + srcW + 4 * colW, y, slW, slH);
                y += rowGap;

                matrix266SmoothPitchSlider.setBounds        (innerX + srcW + 0 * colW, y, slW, slH);
                matrix266SmoothFoldSlider.setBounds         (innerX + srcW + 1 * colW, y, slW, slH);
                matrix266SmoothLpgSlider.setBounds          (innerX + srcW + 2 * colW, y, slW, slH);
                matrix266SmoothMod281AttackSlider.setBounds (innerX + srcW + 3 * colW, y, slW, slH);
                matrix266SmoothMod281DecaySlider.setBounds  (innerX + srcW + 4 * colW, y, slW, slH);
                y += rowGap;

                matrix266SteppedPitchSlider.setBounds       (innerX + srcW + 0 * colW, y, slW, slH);
                matrix266SteppedFoldSlider.setBounds        (innerX + srcW + 1 * colW, y, slW, slH);
                matrix266SteppedLpgSlider.setBounds         (innerX + srcW + 2 * colW, y, slW, slH);
                matrix266SteppedMod281AttackSlider.setBounds(innerX + srcW + 3 * colW, y, slW, slH);
                matrix266SteppedMod281DecaySlider.setBounds (innerX + srcW + 4 * colW, y, slW, slH);
    }
}
