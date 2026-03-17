#include "PluginEditor.h"

WestPatchAudioProcessorEditor::WestPatchAudioProcessorEditor (WestPatchAudioProcessor& p)
    : AudioProcessorEditor (p), audioProcessor (p)
{
    setSize (1120, 820);

    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible (label);
    };

    auto setupMatrixSlider = [this](juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::LinearVertical);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRange (-1.0, 1.0, 0.01);
        slider.setColour (juce::Slider::trackColourId, juce::Colours::orange);
        slider.setColour (juce::Slider::thumbColourId, juce::Colours::white);
        addAndMakeVisible (slider);
    };

    setupSlider (foldSlider, foldLabel, "Fold");
    setupSlider (attackSlider, attackLabel, "Attack");
    setupSlider (releaseSlider, releaseLabel, "Release");
    setupSlider (lpgSlider, lpgLabel, "LPG");

    setupSlider (funcBRateSlider, funcBRateLabel, "281 Rate");
    setupSlider (funcBDepthSlider, funcBDepthLabel, "281 Depth");

    setupSlider (uncertaintyRateSlider, uncertaintyRateLabel, "266 Rate");
    setupSlider (uncertaintySmoothSlider, uncertaintySmoothLabel, "266 Smooth");
    setupSlider (uncertaintySteppedSlider, uncertaintySteppedLabel, "266 Stepped");

    setupSlider (synthLevelSlider, synthLevelLabel, "Synth");
    setupSlider (inputLevelSlider, inputLevelLabel, "Input");
    setupSlider (noiseLevelSlider, noiseLevelLabel, "Noise");

    setupSlider (test266SmoothToFoldSlider, test266SmoothToFoldLabel, "266→Fold");
    setupSlider (test266SteppedToPitchSlider, test266SteppedToPitchLabel, "266→Pitch");
    setupSlider (test266BiasToLpgSlider, test266BiasToLpgLabel, "266→LPG");
    setupSlider (test266PulseToTriggerSlider, test266PulseToTriggerLabel, "266 Pulse");

    setupSlider (detuneAmountSlider, detuneAmountLabel, "Detune");
    setupSlider (stereoSpreadSlider, stereoSpreadLabel, "Spread");

    setupSlider (complexModRatioSlider, complexModRatioLabel, "Mod Ratio");
    setupSlider (complexFmAmountSlider, complexFmAmountLabel, "FM Amt");
    setupSlider (complexOscMixSlider, complexOscMixLabel, "Osc Mix");

    setupMatrixSlider (matrix281PitchSlider);
    setupMatrixSlider (matrix281FoldSlider);
    setupMatrixSlider (matrix281LpgSlider);

    setupMatrixSlider (matrix266SmoothPitchSlider);
    setupMatrixSlider (matrix266SmoothFoldSlider);
    setupMatrixSlider (matrix266SmoothLpgSlider);

    setupMatrixSlider (matrix266SteppedPitchSlider);
    setupMatrixSlider (matrix266SteppedFoldSlider);
    setupMatrixSlider (matrix266SteppedLpgSlider);

    funcBCycleButton.setButtonText ("281 Cycle");
    funcBCycleButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    funcBCycleButton.setToggleState (audioProcessor.funcBCycle, juce::dontSendNotification);
    addAndMakeVisible (funcBCycleButton);

    gateInputButton.setButtonText ("Gate Input");
    gateInputButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    gateInputButton.setToggleState (audioProcessor.gateExternalInput, juce::dontSendNotification);
    addAndMakeVisible (gateInputButton);

    matrixTitleLabel.setText ("Mod Matrix", juce::dontSendNotification);
    matrixTitleLabel.setJustificationType (juce::Justification::centredLeft);
    matrixTitleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (matrixTitleLabel);

    matrixPitchLabel.setText ("Pitch", juce::dontSendNotification);
    matrixPitchLabel.setJustificationType (juce::Justification::centred);
    matrixPitchLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (matrixPitchLabel);

    matrixFoldLabel.setText ("Fold", juce::dontSendNotification);
    matrixFoldLabel.setJustificationType (juce::Justification::centred);
    matrixFoldLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (matrixFoldLabel);

    matrixLpgLabel.setText ("LPG", juce::dontSendNotification);
    matrixLpgLabel.setJustificationType (juce::Justification::centred);
    matrixLpgLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (matrixLpgLabel);

    matrix281Label.setText ("281", juce::dontSendNotification);
    matrix281Label.setJustificationType (juce::Justification::centredRight);
    matrix281Label.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (matrix281Label);

    matrix266SmoothLabel.setText ("266 Smooth", juce::dontSendNotification);
    matrix266SmoothLabel.setJustificationType (juce::Justification::centredRight);
    matrix266SmoothLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (matrix266SmoothLabel);

    matrix266SteppedLabel.setText ("266 Stepped", juce::dontSendNotification);
    matrix266SteppedLabel.setJustificationType (juce::Justification::centredRight);
    matrix266SteppedLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (matrix266SteppedLabel);

    foldSlider.setRange (1.0, 6.0, 0.01);
    attackSlider.setRange (0.001, 1.0, 0.001);
    releaseSlider.setRange (0.01, 2.0, 0.001);
    lpgSlider.setRange (0.01, 0.5, 0.001);

    funcBRateSlider.setRange (0.05, 20.0, 0.01);
    funcBDepthSlider.setRange (0.0, 200.0, 0.1);

    uncertaintyRateSlider.setRange (0.05, 10.0, 0.01);
    uncertaintySmoothSlider.setRange (0.0, 3.0, 0.01);
    uncertaintySteppedSlider.setRange (0.0, 50.0, 0.1);

    synthLevelSlider.setRange (0.0, 1.5, 0.01);
    inputLevelSlider.setRange (0.0, 1.5, 0.01);
    noiseLevelSlider.setRange (0.0, 1.5, 0.01);

    test266SmoothToFoldSlider.setRange (0.0, 2.0, 0.01);
    test266SteppedToPitchSlider.setRange (0.0, 1.0, 0.01);
    test266BiasToLpgSlider.setRange (0.0, 1.0, 0.01);
    test266PulseToTriggerSlider.setRange (0.0, 1.0, 0.01);

    detuneAmountSlider.setRange (0.0, 1.0, 0.01);
    stereoSpreadSlider.setRange (0.0, 1.0, 0.01);

    complexModRatioSlider.setRange (0.25, 8.0, 0.01);
    complexFmAmountSlider.setRange (0.0, 300.0, 0.1);
    complexOscMixSlider.setRange (0.0, 1.0, 0.01);

    foldSlider.setValue (audioProcessor.foldAmount);
    attackSlider.setValue (audioProcessor.attackTime);
    releaseSlider.setValue (audioProcessor.releaseTime);
    lpgSlider.setValue (audioProcessor.lpgAmount);

    funcBRateSlider.setValue (audioProcessor.funcBRate);
    funcBDepthSlider.setValue (audioProcessor.funcBDepth);

    uncertaintyRateSlider.setValue (audioProcessor.uncertaintyRate);
    uncertaintySmoothSlider.setValue (audioProcessor.uncertaintySmoothDepth);
    uncertaintySteppedSlider.setValue (audioProcessor.uncertaintySteppedDepth);

    synthLevelSlider.setValue (audioProcessor.synthLevel);
    inputLevelSlider.setValue (audioProcessor.inputLevel);
    noiseLevelSlider.setValue (audioProcessor.noiseLevel);

    test266SmoothToFoldSlider.setValue (audioProcessor.test266SmoothToFold);
    test266SteppedToPitchSlider.setValue (audioProcessor.test266SteppedToPitch);
    test266BiasToLpgSlider.setValue (audioProcessor.test266BiasToLpg);
    test266PulseToTriggerSlider.setValue (audioProcessor.test266PulseToTrigger);

    detuneAmountSlider.setValue (audioProcessor.detuneAmount);
    stereoSpreadSlider.setValue (audioProcessor.stereoSpread);

    complexModRatioSlider.setValue (audioProcessor.complexModRatio);
    complexFmAmountSlider.setValue (audioProcessor.complexFmAmount);
    complexOscMixSlider.setValue (audioProcessor.complexOscMix);

    matrix281PitchSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Pitch]);
    matrix281FoldSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Fold]);
    matrix281LpgSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::LPG]);

    matrix266SmoothPitchSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Pitch]);
    matrix266SmoothFoldSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Fold]);
    matrix266SmoothLpgSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::LPG]);

    matrix266SteppedPitchSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Pitch]);
    matrix266SteppedFoldSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Fold]);
    matrix266SteppedLpgSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::LPG]);

    foldSlider.onValueChange = [this] { audioProcessor.foldAmount = (float) foldSlider.getValue(); };
    attackSlider.onValueChange = [this] { audioProcessor.attackTime = (float) attackSlider.getValue(); };
    releaseSlider.onValueChange = [this] { audioProcessor.releaseTime = (float) releaseSlider.getValue(); };
    lpgSlider.onValueChange = [this] { audioProcessor.lpgAmount = (float) lpgSlider.getValue(); };

    funcBRateSlider.onValueChange = [this] { audioProcessor.funcBRate = (float) funcBRateSlider.getValue(); };
    funcBDepthSlider.onValueChange = [this] { audioProcessor.funcBDepth = (float) funcBDepthSlider.getValue(); };
    funcBCycleButton.onClick = [this] { audioProcessor.funcBCycle = funcBCycleButton.getToggleState(); };

    uncertaintyRateSlider.onValueChange = [this] { audioProcessor.uncertaintyRate = (float) uncertaintyRateSlider.getValue(); };
    uncertaintySmoothSlider.onValueChange = [this] { audioProcessor.uncertaintySmoothDepth = (float) uncertaintySmoothSlider.getValue(); };
    uncertaintySteppedSlider.onValueChange = [this] { audioProcessor.uncertaintySteppedDepth = (float) uncertaintySteppedSlider.getValue(); };

    synthLevelSlider.onValueChange = [this] { audioProcessor.synthLevel = (float) synthLevelSlider.getValue(); };
    inputLevelSlider.onValueChange = [this] { audioProcessor.inputLevel = (float) inputLevelSlider.getValue(); };
    noiseLevelSlider.onValueChange = [this] { audioProcessor.noiseLevel = (float) noiseLevelSlider.getValue(); };
    gateInputButton.onClick = [this] { audioProcessor.gateExternalInput = gateInputButton.getToggleState(); };

    test266SmoothToFoldSlider.onValueChange = [this] { audioProcessor.test266SmoothToFold = (float) test266SmoothToFoldSlider.getValue(); };
    test266SteppedToPitchSlider.onValueChange = [this] { audioProcessor.test266SteppedToPitch = (float) test266SteppedToPitchSlider.getValue(); };
    test266BiasToLpgSlider.onValueChange = [this] { audioProcessor.test266BiasToLpg = (float) test266BiasToLpgSlider.getValue(); };
    test266PulseToTriggerSlider.onValueChange = [this] { audioProcessor.test266PulseToTrigger = (float) test266PulseToTriggerSlider.getValue(); };

    detuneAmountSlider.onValueChange = [this] { audioProcessor.detuneAmount = (float) detuneAmountSlider.getValue(); };
    stereoSpreadSlider.onValueChange = [this] { audioProcessor.stereoSpread = (float) stereoSpreadSlider.getValue(); };

    complexModRatioSlider.onValueChange = [this] { audioProcessor.complexModRatio = (float) complexModRatioSlider.getValue(); };
    complexFmAmountSlider.onValueChange = [this] { audioProcessor.complexFmAmount = (float) complexFmAmountSlider.getValue(); };
    complexOscMixSlider.onValueChange = [this] { audioProcessor.complexOscMix = (float) complexOscMixSlider.getValue(); };

    matrix281PitchSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Pitch]
            = (float) matrix281PitchSlider.getValue();
    };

    matrix281FoldSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Fold]
            = (float) matrix281FoldSlider.getValue();
    };

    matrix281LpgSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::LPG]
            = (float) matrix281LpgSlider.getValue();
    };

    matrix266SmoothPitchSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Pitch]
            = (float) matrix266SmoothPitchSlider.getValue();
    };

    matrix266SmoothFoldSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Fold]
            = (float) matrix266SmoothFoldSlider.getValue();
    };

    matrix266SmoothLpgSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::LPG]
            = (float) matrix266SmoothLpgSlider.getValue();
    };

    matrix266SteppedPitchSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Pitch]
            = (float) matrix266SteppedPitchSlider.getValue();
    };

    matrix266SteppedFoldSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Fold]
            = (float) matrix266SteppedFoldSlider.getValue();
    };

    matrix266SteppedLpgSlider.onValueChange = [this]
    {
        audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::LPG]
            = (float) matrix266SteppedLpgSlider.getValue();
    };
}

WestPatchAudioProcessorEditor::~WestPatchAudioProcessorEditor() = default;

void WestPatchAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (18, 18, 18));

    g.setColour (juce::Colours::orange);
    g.setFont (28.0f);
    g.drawFittedText ("WestPatch Semi", 0, 14, getWidth(), 34, juce::Justification::centred, 1);

    g.setColour (juce::Colours::darkgrey);
    g.drawRoundedRectangle (20.0f, 80.0f, 1080.0f, 720.0f, 16.0f, 2.0f);

    g.setColour (juce::Colours::lightgrey);
    g.setFont (16.0f);
    g.drawText ("Voice", 50, 95, 180, 22, juce::Justification::left);
    g.drawText ("281 Function B", 50, 285, 220, 22, juce::Justification::left);
    g.drawText ("266 Uncertainty", 430, 285, 220, 22, juce::Justification::left);
    g.drawText ("Mixer", 540, 95, 180, 22, juce::Justification::left);
    g.drawText ("266 Test Routing", 50, 475, 220, 22, juce::Justification::left);
    g.drawText ("Lane Width", 540, 475, 180, 22, juce::Justification::left);
    g.drawText ("Complex Osc", 50, 655, 220, 22, juce::Justification::left);
}

void WestPatchAudioProcessorEditor::resized()
{
    const int sliderSize = 90;
    const int labelHeight = 20;
    const int gap = 14;
    const int left = 50;

    const int row1Y = 130;
    const int row2Y = 320;
    const int row3Y = 510;
    const int row4Y = 690;

    foldSlider.setBounds    (left + 0 * (sliderSize + gap), row1Y, sliderSize, sliderSize);
    attackSlider.setBounds  (left + 1 * (sliderSize + gap), row1Y, sliderSize, sliderSize);
    releaseSlider.setBounds (left + 2 * (sliderSize + gap), row1Y, sliderSize, sliderSize);
    lpgSlider.setBounds     (left + 3 * (sliderSize + gap), row1Y, sliderSize, sliderSize);

    funcBRateSlider.setBounds  (left + 0 * (sliderSize + gap), row2Y, sliderSize, sliderSize);
    funcBDepthSlider.setBounds (left + 1 * (sliderSize + gap), row2Y, sliderSize, sliderSize);

    uncertaintyRateSlider.setBounds    (left + 4 * (sliderSize + gap), row2Y, sliderSize, sliderSize);
    uncertaintySmoothSlider.setBounds  (left + 5 * (sliderSize + gap), row2Y, sliderSize, sliderSize);
    uncertaintySteppedSlider.setBounds (left + 6 * (sliderSize + gap), row2Y, sliderSize, sliderSize);

    test266SmoothToFoldSlider.setBounds   (left + 0 * (sliderSize + gap), row3Y, sliderSize, sliderSize);
    test266SteppedToPitchSlider.setBounds (left + 1 * (sliderSize + gap), row3Y, sliderSize, sliderSize);
    test266BiasToLpgSlider.setBounds      (left + 2 * (sliderSize + gap), row3Y, sliderSize, sliderSize);
    test266PulseToTriggerSlider.setBounds (left + 3 * (sliderSize + gap), row3Y, sliderSize, sliderSize);

    detuneAmountSlider.setBounds (540, 510, sliderSize, sliderSize);
    stereoSpreadSlider.setBounds (540 + (sliderSize + gap), 510, sliderSize, sliderSize);

    complexModRatioSlider.setBounds (left + 0 * (sliderSize + gap), row4Y, sliderSize, sliderSize);
    complexFmAmountSlider.setBounds (left + 1 * (sliderSize + gap), row4Y, sliderSize, sliderSize);
    complexOscMixSlider.setBounds   (left + 2 * (sliderSize + gap), row4Y, sliderSize, sliderSize);

    foldLabel.setBounds    (foldSlider.getX(), foldSlider.getBottom(), sliderSize, labelHeight);
    attackLabel.setBounds  (attackSlider.getX(), attackSlider.getBottom(), sliderSize, labelHeight);
    releaseLabel.setBounds (releaseSlider.getX(), releaseSlider.getBottom(), sliderSize, labelHeight);
    lpgLabel.setBounds     (lpgSlider.getX(), lpgSlider.getBottom(), sliderSize, labelHeight);

    funcBRateLabel.setBounds  (funcBRateSlider.getX(), funcBRateSlider.getBottom(), sliderSize, labelHeight);
    funcBDepthLabel.setBounds (funcBDepthSlider.getX(), funcBDepthSlider.getBottom(), sliderSize, labelHeight);

    uncertaintyRateLabel.setBounds    (uncertaintyRateSlider.getX(), uncertaintyRateSlider.getBottom(), sliderSize, labelHeight);
    uncertaintySmoothLabel.setBounds  (uncertaintySmoothSlider.getX(), uncertaintySmoothSlider.getBottom(), sliderSize, labelHeight);
    uncertaintySteppedLabel.setBounds (uncertaintySteppedSlider.getX(), uncertaintySteppedSlider.getBottom(), sliderSize, labelHeight);

    test266SmoothToFoldLabel.setBounds   (test266SmoothToFoldSlider.getX(), test266SmoothToFoldSlider.getBottom(), sliderSize, labelHeight);
    test266SteppedToPitchLabel.setBounds (test266SteppedToPitchSlider.getX(), test266SteppedToPitchSlider.getBottom(), sliderSize, labelHeight);
    test266BiasToLpgLabel.setBounds      (test266BiasToLpgSlider.getX(), test266BiasToLpgSlider.getBottom(), sliderSize, labelHeight);
    test266PulseToTriggerLabel.setBounds (test266PulseToTriggerSlider.getX(), test266PulseToTriggerSlider.getBottom(), sliderSize, labelHeight);

    detuneAmountLabel.setBounds (detuneAmountSlider.getX(), detuneAmountSlider.getBottom(), sliderSize, labelHeight);
    stereoSpreadLabel.setBounds (stereoSpreadSlider.getX(), stereoSpreadSlider.getBottom(), sliderSize, labelHeight);

    complexModRatioLabel.setBounds (complexModRatioSlider.getX(), complexModRatioSlider.getBottom(), sliderSize, labelHeight);
    complexFmAmountLabel.setBounds (complexFmAmountSlider.getX(), complexFmAmountSlider.getBottom(), sliderSize, labelHeight);
    complexOscMixLabel.setBounds   (complexOscMixSlider.getX(), complexOscMixSlider.getBottom(), sliderSize, labelHeight);

    funcBCycleButton.setBounds (left + 2 * (sliderSize + gap), row2Y + 28, 130, 24);

    synthLevelSlider.setBounds (540, 130, sliderSize, sliderSize);
    inputLevelSlider.setBounds (540 + (sliderSize + gap), 130, sliderSize, sliderSize);
    noiseLevelSlider.setBounds (540 + 2 * (sliderSize + gap), 130, sliderSize, sliderSize);

    synthLevelLabel.setBounds (synthLevelSlider.getX(), synthLevelSlider.getBottom(), sliderSize, labelHeight);
    inputLevelLabel.setBounds (inputLevelSlider.getX(), inputLevelSlider.getBottom(), sliderSize, labelHeight);
    noiseLevelLabel.setBounds (noiseLevelSlider.getX(), noiseLevelSlider.getBottom(), sliderSize, labelHeight);

    gateInputButton.setBounds (640, 250, 130, 24);

    matrixTitleLabel.setBounds (800, 500, 120, 24);
    matrixPitchLabel.setBounds (900, 500, 40, 24);
    matrixFoldLabel.setBounds  (960, 500, 40, 24);
    matrixLpgLabel.setBounds   (1020, 500, 40, 24);

    matrix281Label.setBounds        (790, 535, 100, 24);
    matrix266SmoothLabel.setBounds  (790, 590, 100, 24);
    matrix266SteppedLabel.setBounds (790, 645, 100, 24);

    matrix281PitchSlider.setBounds        (900, 530, 30, 50);
    matrix281FoldSlider.setBounds         (960, 530, 30, 50);
    matrix281LpgSlider.setBounds          (1020, 530, 30, 50);

    matrix266SmoothPitchSlider.setBounds  (900, 585, 30, 50);
    matrix266SmoothFoldSlider.setBounds   (960, 585, 30, 50);
    matrix266SmoothLpgSlider.setBounds    (1020, 585, 30, 50);

    matrix266SteppedPitchSlider.setBounds (900, 640, 30, 50);
    matrix266SteppedFoldSlider.setBounds  (960, 640, 30, 50);
    matrix266SteppedLpgSlider.setBounds   (1020, 640, 30, 50);
}
