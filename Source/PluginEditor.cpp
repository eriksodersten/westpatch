#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <WestPatchBinaryData.h>

//==============================================================================
WestPatchAudioProcessorEditor::WestPatchAudioProcessorEditor (WestPatchAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1280, 760);

    backgroundImage = juce::ImageFileFormat::loadFrom (
        WestPatchBinaryData::background_jpg,
        WestPatchBinaryData::background_jpgSize
    );

    //==========================================================================
    // Group
    setupLabel (groupModeLabel, "Group", juce::Justification::centredLeft);
    addAndMakeVisible (groupModeLabel);

    groupModeBox.addItem ("Unison", 1);
    groupModeBox.addItem ("Duo", 2);
    groupModeBox.addItem ("Quad", 3);

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

    //==========================================================================
    // Voice
    setupLabel (attackLabel, "Attack");
    addAndMakeVisible (attackLabel);
    setupKnob (attackSlider, 0.001, 5.0, 0.001);
    attackSlider.setValue (audioProcessor.attackTime, juce::dontSendNotification);
    attackSlider.onValueChange = [this] { audioProcessor.attackTime = (float) attackSlider.getValue(); };
    addAndMakeVisible (attackSlider);

    setupLabel (releaseLabel, "Release");
    addAndMakeVisible (releaseLabel);
    setupKnob (releaseSlider, 0.001, 5.0, 0.001);
    releaseSlider.setValue (audioProcessor.releaseTime, juce::dontSendNotification);
    releaseSlider.onValueChange = [this] { audioProcessor.releaseTime = (float) releaseSlider.getValue(); };
    addAndMakeVisible (releaseSlider);

    setupLabel (foldLabel, "Fold");
    addAndMakeVisible (foldLabel);
    setupKnob (foldSlider, 0.0, 5.0, 0.01);
        foldSlider.setValue (audioProcessor.foldAmount - 0.05, juce::dontSendNotification);
        foldSlider.onValueChange = [this] { audioProcessor.foldAmount = (float) foldSlider.getValue() + 0.05f; };
    addAndMakeVisible (foldSlider);

    setupLabel (lpgLabel, "LPG");
    addAndMakeVisible (lpgLabel);
    setupKnob (lpgSlider, 0.0, 1.0, 0.001);
    lpgSlider.setValue (audioProcessor.lpgAmount, juce::dontSendNotification);
    lpgSlider.onValueChange = [this] { audioProcessor.lpgAmount = (float) lpgSlider.getValue(); };
    addAndMakeVisible (lpgSlider);

    //==========================================================================
    // 281
    setupLabel (funcBRateLabel, "281 Rate");
    addAndMakeVisible (funcBRateLabel);
    setupKnob (funcBRateSlider, 0.01, 20.0, 0.01);
    funcBRateSlider.setValue (audioProcessor.funcBRate, juce::dontSendNotification);
    funcBRateSlider.onValueChange = [this] { audioProcessor.funcBRate = (float) funcBRateSlider.getValue(); };
    addAndMakeVisible (funcBRateSlider);

    setupLabel (funcBDepthLabel, "281 Depth");
    addAndMakeVisible (funcBDepthLabel);
    setupKnob (funcBDepthSlider, 0.0, 48.0, 0.01);
    funcBDepthSlider.setValue (audioProcessor.funcBDepth, juce::dontSendNotification);
    funcBDepthSlider.onValueChange = [this] { audioProcessor.funcBDepth = (float) funcBDepthSlider.getValue(); };
    addAndMakeVisible (funcBDepthSlider);

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

    //==========================================================================
    // 266
    setupLabel (uncertaintyRateLabel, "266 Rate");
    addAndMakeVisible (uncertaintyRateLabel);
    setupKnob (uncertaintyRateSlider, 0.01, 20.0, 0.01);
    uncertaintyRateSlider.setValue (audioProcessor.uncertaintyRate, juce::dontSendNotification);
    uncertaintyRateSlider.onValueChange = [this] { audioProcessor.uncertaintyRate = (float) uncertaintyRateSlider.getValue(); };
    addAndMakeVisible (uncertaintyRateSlider);

    setupLabel (uncertaintySmoothLabel, "266 Smooth");
    addAndMakeVisible (uncertaintySmoothLabel);
    setupKnob (uncertaintySmoothSlider, 0.0, 1.0, 0.001);
    uncertaintySmoothSlider.setValue (audioProcessor.uncertaintySmoothDepth, juce::dontSendNotification);
    uncertaintySmoothSlider.onValueChange = [this] { audioProcessor.uncertaintySmoothDepth = (float) uncertaintySmoothSlider.getValue(); };
    addAndMakeVisible (uncertaintySmoothSlider);

    setupLabel (uncertaintySteppedLabel, "266 Stepped");
    addAndMakeVisible (uncertaintySteppedLabel);
    setupKnob (uncertaintySteppedSlider, 0.0, 24.0, 0.01);
    uncertaintySteppedSlider.setValue (audioProcessor.uncertaintySteppedDepth, juce::dontSendNotification);
    uncertaintySteppedSlider.onValueChange = [this] { audioProcessor.uncertaintySteppedDepth = (float) uncertaintySteppedSlider.getValue(); };
    addAndMakeVisible (uncertaintySteppedSlider);

    //==========================================================================
    // Mixer
    setupLabel (synthLevelLabel, "Synth");
    addAndMakeVisible (synthLevelLabel);
    setupKnob (synthLevelSlider, 0.0, 1.0, 0.001);
    synthLevelSlider.setValue (audioProcessor.synthLevel, juce::dontSendNotification);
    synthLevelSlider.onValueChange = [this] { audioProcessor.synthLevel = (float) synthLevelSlider.getValue(); };
    addAndMakeVisible (synthLevelSlider);

    setupLabel (inputLevelLabel, "Input");
    addAndMakeVisible (inputLevelLabel);
    setupKnob (inputLevelSlider, 0.0, 1.0, 0.001);
    inputLevelSlider.setValue (audioProcessor.inputLevel, juce::dontSendNotification);
    inputLevelSlider.onValueChange = [this] { audioProcessor.inputLevel = (float) inputLevelSlider.getValue(); };
    addAndMakeVisible (inputLevelSlider);

    setupLabel (noiseLevelLabel, "Noise");
    addAndMakeVisible (noiseLevelLabel);
    setupKnob (noiseLevelSlider, 0.0, 1.0, 0.001);
    noiseLevelSlider.setValue (audioProcessor.noiseLevel, juce::dontSendNotification);
    noiseLevelSlider.onValueChange = [this] { audioProcessor.noiseLevel = (float) noiseLevelSlider.getValue(); };
    addAndMakeVisible (noiseLevelSlider);

    gateInputButton.setButtonText ("Gate Input");
    gateInputButton.setToggleState (audioProcessor.gateExternalInput, juce::dontSendNotification);
    gateInputButton.onClick = [this] { audioProcessor.gateExternalInput = gateInputButton.getToggleState(); };
    addAndMakeVisible (gateInputButton);

    //==========================================================================
    // 266 direct routing / tests
    setupLabel (test266SmoothToFoldLabel, "266 S->Fold");
    addAndMakeVisible (test266SmoothToFoldLabel);
    setupKnob (test266SmoothToFoldSlider, 0.0, 1.0, 0.001);
    test266SmoothToFoldSlider.setValue (audioProcessor.test266SmoothToFold, juce::dontSendNotification);
    test266SmoothToFoldSlider.onValueChange = [this] { audioProcessor.test266SmoothToFold = (float) test266SmoothToFoldSlider.getValue(); };
    addAndMakeVisible (test266SmoothToFoldSlider);

    setupLabel (test266SteppedToPitchLabel, "266 St->Pitch");
    addAndMakeVisible (test266SteppedToPitchLabel);
    setupKnob (test266SteppedToPitchSlider, 0.0, 1.0, 0.001);
    test266SteppedToPitchSlider.setValue (audioProcessor.test266SteppedToPitch, juce::dontSendNotification);
    test266SteppedToPitchSlider.onValueChange = [this] { audioProcessor.test266SteppedToPitch = (float) test266SteppedToPitchSlider.getValue(); };
    addAndMakeVisible (test266SteppedToPitchSlider);

    setupLabel (test266BiasToLpgLabel, "266 Bias->LPG");
    addAndMakeVisible (test266BiasToLpgLabel);
    setupKnob (test266BiasToLpgSlider, 0.0, 1.0, 0.001);
    test266BiasToLpgSlider.setValue (audioProcessor.test266BiasToLpg, juce::dontSendNotification);
    test266BiasToLpgSlider.onValueChange = [this] { audioProcessor.test266BiasToLpg = (float) test266BiasToLpgSlider.getValue(); };
    addAndMakeVisible (test266BiasToLpgSlider);

    setupLabel (test266PulseToTriggerLabel, "266 Pulse->Trig");
    addAndMakeVisible (test266PulseToTriggerLabel);
    setupKnob (test266PulseToTriggerSlider, 0.0, 1.0, 0.001);
    test266PulseToTriggerSlider.setValue (audioProcessor.test266PulseToTrigger, juce::dontSendNotification);
    test266PulseToTriggerSlider.onValueChange = [this] { audioProcessor.test266PulseToTrigger = (float) test266PulseToTriggerSlider.getValue(); };
    addAndMakeVisible (test266PulseToTriggerSlider);

    //==========================================================================
    // Lane / complex
    setupLabel (detuneAmountLabel, "Detune");
    addAndMakeVisible (detuneAmountLabel);
    setupKnob (detuneAmountSlider, 0.0, 1.0, 0.001);
    detuneAmountSlider.setValue (audioProcessor.detuneAmount, juce::dontSendNotification);
    detuneAmountSlider.onValueChange = [this] { audioProcessor.detuneAmount = (float) detuneAmountSlider.getValue(); };
    addAndMakeVisible (detuneAmountSlider);

    setupLabel (stereoSpreadLabel, "Spread");
    addAndMakeVisible (stereoSpreadLabel);
    setupKnob (stereoSpreadSlider, 0.0, 1.0, 0.001);
    stereoSpreadSlider.setValue (audioProcessor.stereoSpread, juce::dontSendNotification);
    stereoSpreadSlider.onValueChange = [this] { audioProcessor.stereoSpread = (float) stereoSpreadSlider.getValue(); };
    addAndMakeVisible (stereoSpreadSlider);

    setupLabel (complexModRatioLabel, "Mod Ratio");
    addAndMakeVisible (complexModRatioLabel);
    setupKnob (complexModRatioSlider, 0.1, 8.0, 0.01);
    complexModRatioSlider.setValue (audioProcessor.complexModRatio, juce::dontSendNotification);
    complexModRatioSlider.onValueChange = [this] { audioProcessor.complexModRatio = (float) complexModRatioSlider.getValue(); };
    addAndMakeVisible (complexModRatioSlider);

    setupLabel (complexFmAmountLabel, "FM Amount");
    addAndMakeVisible (complexFmAmountLabel);
    setupKnob (complexFmAmountSlider, 0.0, 100.0, 0.01);
    complexFmAmountSlider.setValue (audioProcessor.complexFmAmount, juce::dontSendNotification);
    complexFmAmountSlider.onValueChange = [this] { audioProcessor.complexFmAmount = (float) complexFmAmountSlider.getValue(); };
    addAndMakeVisible (complexFmAmountSlider);

    setupLabel (complexOscMixLabel, "Osc Mix");
    addAndMakeVisible (complexOscMixLabel);
    setupKnob (complexOscMixSlider, 0.0, 1.0, 0.001);
    complexOscMixSlider.setValue (audioProcessor.complexOscMix, juce::dontSendNotification);
    complexOscMixSlider.onValueChange = [this] { audioProcessor.complexOscMix = (float) complexOscMixSlider.getValue(); };
    addAndMakeVisible (complexOscMixSlider);

    //==========================================================================
    // Matrix
    setupLabel (matrixTitleLabel, "Mod Matrix", juce::Justification::centredLeft);
    addAndMakeVisible (matrixTitleLabel);

    setupLabel (matrixSource281Label, "281", juce::Justification::centredLeft);
    addAndMakeVisible (matrixSource281Label);

    setupLabel (matrixSource266SmoothLabel, "266 Smooth", juce::Justification::centredLeft);
    addAndMakeVisible (matrixSource266SmoothLabel);

    setupLabel (matrixSource266SteppedLabel, "266 Stepped", juce::Justification::centredLeft);
    addAndMakeVisible (matrixSource266SteppedLabel);

    setupLabel (matrixPitchLabel, "Pitch");
    addAndMakeVisible (matrixPitchLabel);

    setupLabel (matrixFoldLabel, "Fold");
    addAndMakeVisible (matrixFoldLabel);

    setupLabel (matrixLpgLabel, "LPG");
    addAndMakeVisible (matrixLpgLabel);

    setupMatrixSlider (matrix281PitchSlider);
    matrix281PitchSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Pitch], juce::dontSendNotification);
    matrix281PitchSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Pitch]
            = (float) matrix281PitchSlider.getValue();
    };
    addAndMakeVisible (matrix281PitchSlider);

    setupMatrixSlider (matrix281FoldSlider);
    matrix281FoldSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Fold], juce::dontSendNotification);
    matrix281FoldSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Fold]
            = (float) matrix281FoldSlider.getValue();
    };
    addAndMakeVisible (matrix281FoldSlider);

    setupMatrixSlider (matrix281LpgSlider);
    matrix281LpgSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::LPG], juce::dontSendNotification);
    matrix281LpgSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::LPG]
            = (float) matrix281LpgSlider.getValue();
    };
    addAndMakeVisible (matrix281LpgSlider);

    setupMatrixSlider (matrix266SmoothPitchSlider);
    matrix266SmoothPitchSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Pitch], juce::dontSendNotification);
    matrix266SmoothPitchSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Pitch]
            = (float) matrix266SmoothPitchSlider.getValue();
    };
    addAndMakeVisible (matrix266SmoothPitchSlider);

    setupMatrixSlider (matrix266SmoothFoldSlider);
    matrix266SmoothFoldSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Fold], juce::dontSendNotification);
    matrix266SmoothFoldSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Fold]
            = (float) matrix266SmoothFoldSlider.getValue();
    };
    addAndMakeVisible (matrix266SmoothFoldSlider);

    setupMatrixSlider (matrix266SmoothLpgSlider);
    matrix266SmoothLpgSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::LPG], juce::dontSendNotification);
    matrix266SmoothLpgSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::LPG]
            = (float) matrix266SmoothLpgSlider.getValue();
    };
    addAndMakeVisible (matrix266SmoothLpgSlider);

    setupMatrixSlider (matrix266SteppedPitchSlider);
    matrix266SteppedPitchSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Pitch], juce::dontSendNotification);
    matrix266SteppedPitchSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Pitch]
            = (float) matrix266SteppedPitchSlider.getValue();
    };
    addAndMakeVisible (matrix266SteppedPitchSlider);

    setupMatrixSlider (matrix266SteppedFoldSlider);
    matrix266SteppedFoldSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Fold], juce::dontSendNotification);
    matrix266SteppedFoldSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Fold]
            = (float) matrix266SteppedFoldSlider.getValue();
    };
    addAndMakeVisible (matrix266SteppedFoldSlider);

    setupMatrixSlider (matrix266SteppedLpgSlider);
    matrix266SteppedLpgSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::LPG], juce::dontSendNotification);
    matrix266SteppedLpgSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::LPG]
            = (float) matrix266SteppedLpgSlider.getValue();
    };
    addAndMakeVisible (matrix266SteppedLpgSlider);
}

WestPatchAudioProcessorEditor::~WestPatchAudioProcessorEditor() = default;

//==============================================================================
void WestPatchAudioProcessorEditor::setupKnob (juce::Slider& slider,
                                               double min,
                                               double max,
                                               double interval)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    slider.setRange (min, max, interval);
}

void WestPatchAudioProcessorEditor::setupMatrixSlider (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::LinearVertical);

    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 16);

    slider.setRange (-1.0, 1.0, 0.001);

    // dubbelklick → reset till 0
    slider.setDoubleClickReturnValue(true, 0.0);

    // färger
    slider.setColour(juce::Slider::trackColourId, juce::Colours::orange);
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    slider.setColour(juce::Slider::backgroundColourId, juce::Colours::black);

    // snap-to-zero
    slider.onValueChange = [&slider]()
    {
        const float deadZone = 0.03f;

        if (std::abs(slider.getValue()) < deadZone)
            slider.setValue(0.0, juce::dontSendNotification);
    };
}

void WestPatchAudioProcessorEditor::setupLabel (juce::Label& label,
                                                const juce::String& text,
                                                juce::Justification justification)
{
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (justification);
    label.setColour (juce::Label::textColourId, juce::Colours::white);
}

//==============================================================================
void WestPatchAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    if (backgroundImage.isValid())
    {
        g.drawImage (backgroundImage,
                     getLocalBounds().toFloat(),
                     juce::RectanglePlacement::fillDestination);
    }

    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRect (getLocalBounds());

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (24.0f));
    g.drawFittedText ("WestPatch",
                      24, 16, 220, 28,
                      juce::Justification::centredLeft,
                      1);

    auto panelColour = juce::Colours::black.withAlpha (0.30f);
    auto borderColour = juce::Colours::white.withAlpha (0.16f);

    const juce::Rectangle<int> topBar        (20, 56, getWidth() - 40, 44);
    const juce::Rectangle<int> voicePanel     (20, 112, 420, 250);
    const juce::Rectangle<int> mod281Panel    (456, 112, 250, 250);
    const juce::Rectangle<int> mod266Panel    (722, 112, 250, 250);
    const juce::Rectangle<int> mixerPanel     (988, 112, 272, 250);
    const juce::Rectangle<int> test266Panel   (20, 382, 420, 320);
    const juce::Rectangle<int> complexPanel   (456, 382, 364, 320);
    const juce::Rectangle<int> matrixPanel    (840, 382, 420, 320);

    g.setColour (panelColour);
    g.fillRoundedRectangle (topBar.toFloat(), 16.0f);
    g.fillRoundedRectangle (voicePanel.toFloat(), 18.0f);
    g.fillRoundedRectangle (mod281Panel.toFloat(), 18.0f);
    g.fillRoundedRectangle (mod266Panel.toFloat(), 18.0f);
    g.fillRoundedRectangle (mixerPanel.toFloat(), 18.0f);
    g.fillRoundedRectangle (test266Panel.toFloat(), 18.0f);
    g.fillRoundedRectangle (complexPanel.toFloat(), 18.0f);
    g.fillRoundedRectangle (matrixPanel.toFloat(), 18.0f);

    g.setColour (borderColour);
    g.drawRoundedRectangle (topBar.toFloat(), 16.0f, 1.0f);
    g.drawRoundedRectangle (voicePanel.toFloat(), 18.0f, 1.0f);
    g.drawRoundedRectangle (mod281Panel.toFloat(), 18.0f, 1.0f);
    g.drawRoundedRectangle (mod266Panel.toFloat(), 18.0f, 1.0f);
    g.drawRoundedRectangle (mixerPanel.toFloat(), 18.0f, 1.0f);
    g.drawRoundedRectangle (test266Panel.toFloat(), 18.0f, 1.0f);
    g.drawRoundedRectangle (complexPanel.toFloat(), 18.0f, 1.0f);
    g.drawRoundedRectangle (matrixPanel.toFloat(), 18.0f, 1.0f);

    g.setFont (juce::FontOptions (14.0f));
    g.setColour (juce::Colours::white);
    g.drawFittedText ("Voice",        voicePanel.reduced (14).removeFromTop (20),   juce::Justification::centredLeft, 1);
    g.drawFittedText ("281",          mod281Panel.reduced (14).removeFromTop (20),  juce::Justification::centredLeft, 1);
    g.drawFittedText ("266",          mod266Panel.reduced (14).removeFromTop (20),  juce::Justification::centredLeft, 1);
    g.drawFittedText ("Mixer",        mixerPanel.reduced (14).removeFromTop (20),   juce::Justification::centredLeft, 1);
    g.drawFittedText ("266 Direct",   test266Panel.reduced (14).removeFromTop (20), juce::Justification::centredLeft, 1);
    g.drawFittedText ("Lane / Complex", complexPanel.reduced (14).removeFromTop (20), juce::Justification::centredLeft, 1);
    g.drawFittedText ("Mod Matrix",   matrixPanel.reduced (14).removeFromTop (20),  juce::Justification::centredLeft, 1);
    // center guides for mod matrix
    g.setColour(juce::Colours::white.withAlpha(0.15f));

    auto matrixPanelGuide = juce::Rectangle<int>(840, 382, 420, 320);

    int columnWidth = 92;
    int sliderStartX = matrixPanelGuide.getX() + 18 + 110 + 14;

    for (int i = 0; i < 3; ++i)
    {
        int x = sliderStartX + i * columnWidth + 28;

        g.drawVerticalLine(x,
                           matrixPanelGuide.getY() + 60,
                           matrixPanelGuide.getBottom() - 20);
    }
    
}

void WestPatchAudioProcessorEditor::resized()
{
    // Top bar
    groupModeLabel.setBounds (34, 68, 56, 20);
    groupModeBox.setBounds   (92, 66, 130, 24);

    const int knobW = 90;
    const int knobH = 120;

    // Voice panel
    const int voiceX = 34;
    const int voiceLabelY = 160;
    const int voiceKnobY = 180;
    const int voiceSpacing = 96;

    attackLabel.setBounds  (voiceX + 0 * voiceSpacing, voiceLabelY, knobW, 20);
    attackSlider.setBounds (voiceX + 0 * voiceSpacing, voiceKnobY,  knobW, knobH);

    releaseLabel.setBounds  (voiceX + 1 * voiceSpacing, voiceLabelY, knobW, 20);
    releaseSlider.setBounds (voiceX + 1 * voiceSpacing, voiceKnobY,  knobW, knobH);

    foldLabel.setBounds  (voiceX + 2 * voiceSpacing, voiceLabelY, knobW, 20);
    foldSlider.setBounds (voiceX + 2 * voiceSpacing, voiceKnobY,  knobW, knobH);

    lpgLabel.setBounds  (voiceX + 3 * voiceSpacing, voiceLabelY, knobW, 20);
    lpgSlider.setBounds (voiceX + 3 * voiceSpacing, voiceKnobY,  knobW, knobH);

    // 281 panel
    const int mod281X = 470;
    funcBRateLabel.setBounds  (mod281X + 0, 160, knobW, 20);
    funcBRateSlider.setBounds (mod281X + 0, 180, knobW, knobH);

    funcBDepthLabel.setBounds  (mod281X + 96, 160, knobW, 20);
    funcBDepthSlider.setBounds (mod281X + 96, 180, knobW, knobH);

    func281ModeBox.setBounds (mod281X + 8, 312, 160, 24);

    // 266 panel
    const int mod266X = 736;
    uncertaintyRateLabel.setBounds  (mod266X + 0, 160, knobW, 20);
    uncertaintyRateSlider.setBounds (mod266X + 0, 180, knobW, knobH);

    uncertaintySmoothLabel.setBounds  (mod266X + 96, 160, knobW, 20);
    uncertaintySmoothSlider.setBounds (mod266X + 96, 180, knobW, knobH);

    uncertaintySteppedLabel.setBounds  (mod266X + 192, 160, knobW, 20);
    uncertaintySteppedSlider.setBounds (mod266X + 192, 180, knobW, knobH);

    // Mixer panel
    const int mixerX = 1002;
    synthLevelLabel.setBounds  (mixerX + 0, 160, knobW, 20);
    synthLevelSlider.setBounds (mixerX + 0, 180, knobW, knobH);

    inputLevelLabel.setBounds  (mixerX + 88, 160, knobW, 20);
    inputLevelSlider.setBounds (mixerX + 88, 180, knobW, knobH);

    noiseLevelLabel.setBounds  (mixerX + 176, 160, knobW, 20);
    noiseLevelSlider.setBounds (mixerX + 176, 180, knobW, knobH);

    gateInputButton.setBounds (mixerX + 10, 312, 140, 24);

    // 266 direct panel (bottom left)
    const int testX = 34;
    const int testLabelY = 430;
    const int testKnobY = 450;
    const int testSpacing = 96;

    test266SmoothToFoldLabel.setBounds   (testX + 0 * testSpacing, testLabelY, knobW, 20);
    test266SmoothToFoldSlider.setBounds  (testX + 0 * testSpacing, testKnobY,  knobW, knobH);

    test266SteppedToPitchLabel.setBounds  (testX + 1 * testSpacing, testLabelY, knobW, 20);
    test266SteppedToPitchSlider.setBounds (testX + 1 * testSpacing, testKnobY,  knobW, knobH);

    test266BiasToLpgLabel.setBounds  (testX + 2 * testSpacing, testLabelY, knobW, 20);
    test266BiasToLpgSlider.setBounds (testX + 2 * testSpacing, testKnobY,  knobW, knobH);

    test266PulseToTriggerLabel.setBounds  (testX + 3 * testSpacing, testLabelY, knobW, 20);
    test266PulseToTriggerSlider.setBounds (testX + 3 * testSpacing, testKnobY,  knobW, knobH);

    // Lane / complex panel (bottom middle)
    const int complexX = 470;
    const int complexLabelY = 430;
    const int complexKnobY = 450;
    const int complexSpacing = 68;
    const int complexKnobW = 64;
    const int complexKnobH = 110;

    detuneAmountLabel.setBounds   (complexX + 0 * complexSpacing, complexLabelY, complexKnobW, 20);
    detuneAmountSlider.setBounds  (complexX + 0 * complexSpacing, complexKnobY,  complexKnobW, complexKnobH);

    stereoSpreadLabel.setBounds   (complexX + 1 * complexSpacing, complexLabelY, complexKnobW, 20);
    stereoSpreadSlider.setBounds  (complexX + 1 * complexSpacing, complexKnobY,  complexKnobW, complexKnobH);

    complexModRatioLabel.setBounds   (complexX + 2 * complexSpacing, complexLabelY, complexKnobW, 20);
    complexModRatioSlider.setBounds  (complexX + 2 * complexSpacing, complexKnobY,  complexKnobW, complexKnobH);

    complexFmAmountLabel.setBounds   (complexX + 3 * complexSpacing, complexLabelY, complexKnobW, 20);
    complexFmAmountSlider.setBounds  (complexX + 3 * complexSpacing, complexKnobY,  complexKnobW, complexKnobH);

    complexOscMixLabel.setBounds   (complexX + 4 * complexSpacing, complexLabelY, complexKnobW, 20);
    complexOscMixSlider.setBounds  (complexX + 4 * complexSpacing, complexKnobY,  complexKnobW, complexKnobH);

    // Matrix panel - lower right
    const int matrixPanelX = 840;
    const int matrixPanelY = 382;
    const int matrixInnerX = matrixPanelX + 18;
    const int matrixInnerY = matrixPanelY + 44;

    const int sourceLabelW = 110;
    const int sliderW = 56;
    const int sliderH = 80;
    const int rowGap = 84;
    const int columnW = 92;

    matrixTitleLabel.setBounds (matrixPanelX + 14, matrixPanelY + 6, 140, 20);

    matrixPitchLabel.setBounds (matrixInnerX + sourceLabelW + 0 * columnW, matrixInnerY - 24, columnW, 18);
    matrixFoldLabel.setBounds  (matrixInnerX + sourceLabelW + 1 * columnW, matrixInnerY - 24, columnW, 18);
    matrixLpgLabel.setBounds   (matrixInnerX + sourceLabelW + 2 * columnW, matrixInnerY - 24, columnW, 18);

    matrixSource281Label.setBounds        (matrixInnerX, matrixInnerY + 8, sourceLabelW, 20);
    matrixSource266SmoothLabel.setBounds  (matrixInnerX, matrixInnerY + rowGap + 8, sourceLabelW, 20);
    matrixSource266SteppedLabel.setBounds (matrixInnerX, matrixInnerY + 2 * rowGap + 8, sourceLabelW, 20);

    const int sliderX0 = matrixInnerX + sourceLabelW + 14;
    const int sliderY0 = matrixInnerY;

    matrix281PitchSlider.setBounds (sliderX0 + 0 * columnW, sliderY0, sliderW, sliderH);
    matrix281FoldSlider.setBounds  (sliderX0 + 1 * columnW, sliderY0, sliderW, sliderH);
    matrix281LpgSlider.setBounds   (sliderX0 + 2 * columnW, sliderY0, sliderW, sliderH);

    matrix266SmoothPitchSlider.setBounds (sliderX0 + 0 * columnW, sliderY0 + rowGap, sliderW, sliderH);
    matrix266SmoothFoldSlider.setBounds  (sliderX0 + 1 * columnW, sliderY0 + rowGap, sliderW, sliderH);
    matrix266SmoothLpgSlider.setBounds   (sliderX0 + 2 * columnW, sliderY0 + rowGap, sliderW, sliderH);

    matrix266SteppedPitchSlider.setBounds (sliderX0 + 0 * columnW, sliderY0 + 2 * rowGap, sliderW, sliderH);
    matrix266SteppedFoldSlider.setBounds  (sliderX0 + 1 * columnW, sliderY0 + 2 * rowGap, sliderW, sliderH);
    matrix266SteppedLpgSlider.setBounds   (sliderX0 + 2 * columnW, sliderY0 + 2 * rowGap, sliderW, sliderH);
}
