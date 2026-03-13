#include "PluginEditor.h"

WestPatchAudioProcessorEditor::WestPatchAudioProcessorEditor (WestPatchAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (1040, 820);

    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 22);
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
        addAndMakeVisible (slider);
        slider.setColour (juce::Slider::trackColourId, juce::Colours::orange);
        slider.setColour (juce::Slider::thumbColourId, juce::Colours::white);
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
    noiseLevelSlider.setValue (audioProcessor.noiseLevel);

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
    matrix281PitchSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Pitch]);
    matrix281FoldSlider.setValue  (audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Fold]);
    matrix281LpgSlider.setValue   (audioProcessor.modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::LPG]);

    matrix266SmoothPitchSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Pitch]);
    matrix266SmoothFoldSlider.setValue  (audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Fold]);
    matrix266SmoothLpgSlider.setValue   (audioProcessor.modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::LPG]);

    matrix266SteppedPitchSlider.setValue (audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Pitch]);
    matrix266SteppedFoldSlider.setValue  (audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Fold]);
    matrix266SteppedLpgSlider.setValue   (audioProcessor.modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::LPG]);

    foldSlider.onValueChange = [this]
    {
        audioProcessor.foldAmount = (float) foldSlider.getValue();
    };

    attackSlider.onValueChange = [this]
    {
        audioProcessor.attackTime = (float) attackSlider.getValue();
    };

    releaseSlider.onValueChange = [this]
    {
        audioProcessor.releaseTime = (float) releaseSlider.getValue();
    };

    lpgSlider.onValueChange = [this]
    {
        audioProcessor.lpgAmount = (float) lpgSlider.getValue();
    };

    funcBRateSlider.onValueChange = [this]
    {
        audioProcessor.funcBRate = (float) funcBRateSlider.getValue();
    };

    funcBDepthSlider.onValueChange = [this]
    {
        audioProcessor.funcBDepth = (float) funcBDepthSlider.getValue();
    };

    funcBCycleButton.onClick = [this]
    {
        audioProcessor.funcBCycle = funcBCycleButton.getToggleState();
    };
    
    uncertaintyRateSlider.onValueChange = [this]
    {
        audioProcessor.uncertaintyRate = (float) uncertaintyRateSlider.getValue();
    };

    uncertaintySmoothSlider.onValueChange = [this]
    {
        audioProcessor.uncertaintySmoothDepth = (float) uncertaintySmoothSlider.getValue();
    };

    uncertaintySteppedSlider.onValueChange = [this]
    {
        audioProcessor.uncertaintySteppedDepth = (float) uncertaintySteppedSlider.getValue();
    };
    
    synthLevelSlider.onValueChange = [this]
    {
        audioProcessor.synthLevel = (float) synthLevelSlider.getValue();
    };

    inputLevelSlider.onValueChange = [this]
    {
        audioProcessor.inputLevel = (float) inputLevelSlider.getValue();
    };

    gateInputButton.onClick = [this]
    {
        audioProcessor.gateExternalInput = gateInputButton.getToggleState();
    };
    
    noiseLevelSlider.onValueChange = [this]
    {
        audioProcessor.noiseLevel = (float) noiseLevelSlider.getValue();
    };
    
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
    g.setFont (30.0f);
    g.drawFittedText ("WestPatch Semi", 0, 16, getWidth(), 36, juce::Justification::centred, 1);

    g.setColour (juce::Colours::darkgrey);
    g.drawRoundedRectangle (30.0f, 80.0f, 980.0f, 700.0f, 16.0f, 2.0f);

    g.setColour (juce::Colours::lightgrey);
    g.setFont (18.0f);
    g.drawText ("Voice", 60, 95, 200, 24, juce::Justification::left);
    g.drawText ("281 Function B", 60, 330, 250, 24, juce::Justification::left);
    g.drawText ("266 Uncertainty", 500, 330, 250, 24, juce::Justification::left);
    
    g.drawText ("Mixer", 560, 95, 250, 24, juce::Justification::left);
}

void WestPatchAudioProcessorEditor::resized()
{
    const int sliderSize = 110;
    const int labelHeight = 24;
    const int gap = 20;
    const int left = 60;

    const int row1Y = 140;
    const int row2Y = 370;

    foldSlider.setBounds       (left + 0 * (sliderSize + gap), row1Y, sliderSize, sliderSize);
    attackSlider.setBounds     (left + 1 * (sliderSize + gap), row1Y, sliderSize, sliderSize);
    releaseSlider.setBounds    (left + 2 * (sliderSize + gap), row1Y, sliderSize, sliderSize);
    lpgSlider.setBounds        (left + 3 * (sliderSize + gap), row1Y, sliderSize, sliderSize);

    funcBRateSlider.setBounds  (left + 0 * (sliderSize + gap), row2Y, sliderSize, sliderSize);
    funcBDepthSlider.setBounds (left + 1 * (sliderSize + gap), row2Y, sliderSize, sliderSize);

    uncertaintyRateSlider.setBounds    (left + 4 * (sliderSize + gap), row2Y, sliderSize, sliderSize);
    uncertaintySmoothSlider.setBounds  (left + 5 * (sliderSize + gap), row2Y, sliderSize, sliderSize);
    uncertaintySteppedSlider.setBounds (left + 6 * (sliderSize + gap), row2Y, sliderSize, sliderSize);

    foldLabel.setBounds       (foldSlider.getX(),       foldSlider.getBottom(),       sliderSize, labelHeight);
    attackLabel.setBounds     (attackSlider.getX(),     attackSlider.getBottom(),     sliderSize, labelHeight);
    releaseLabel.setBounds    (releaseSlider.getX(),    releaseSlider.getBottom(),    sliderSize, labelHeight);
    lpgLabel.setBounds        (lpgSlider.getX(),        lpgSlider.getBottom(),        sliderSize, labelHeight);

    funcBRateLabel.setBounds  (funcBRateSlider.getX(),  funcBRateSlider.getBottom(),  sliderSize, labelHeight);
    funcBDepthLabel.setBounds (funcBDepthSlider.getX(), funcBDepthSlider.getBottom(), sliderSize, labelHeight);

    uncertaintyRateLabel.setBounds    (uncertaintyRateSlider.getX(),    uncertaintyRateSlider.getBottom(),    sliderSize, labelHeight);
    uncertaintySmoothLabel.setBounds  (uncertaintySmoothSlider.getX(),  uncertaintySmoothSlider.getBottom(),  sliderSize, labelHeight);
    uncertaintySteppedLabel.setBounds (uncertaintySteppedSlider.getX(), uncertaintySteppedSlider.getBottom(), sliderSize, labelHeight);

    funcBCycleButton.setBounds (left + 2 * (sliderSize + gap), row2Y + 38, 150, 30);
    
    synthLevelSlider.setBounds (560, 140, 110, 110);
    inputLevelSlider.setBounds (690, 140, 110, 110);
    noiseLevelSlider.setBounds (820, 140, 110, 110);

    synthLevelLabel.setBounds (560, 250, 110, 24);
    inputLevelLabel.setBounds (690, 250, 110, 24);
    noiseLevelLabel.setBounds (820, 250, 110, 24);

    gateInputButton.setBounds (690, 290, 140, 30);
    
    matrixTitleLabel.setBounds (560, 520, 140, 24);

    matrixPitchLabel.setBounds (700, 520, 50, 24);
    matrixFoldLabel.setBounds  (770, 520, 50, 24);
    matrixLpgLabel.setBounds   (840, 520, 50, 24);

    matrix281Label.setBounds        (560, 555, 120, 24);
    matrix266SmoothLabel.setBounds  (560, 610, 120, 24);
    matrix266SteppedLabel.setBounds (560, 665, 120, 24);

    matrix281PitchSlider.setBounds        (700, 550, 40, 50);
    matrix281FoldSlider.setBounds         (770, 550, 40, 50);
    matrix281LpgSlider.setBounds          (840, 550, 40, 50);

    matrix266SmoothPitchSlider.setBounds  (700, 605, 40, 50);
    matrix266SmoothFoldSlider.setBounds   (770, 605, 40, 50);
    matrix266SmoothLpgSlider.setBounds    (840, 605, 40, 50);

    matrix266SteppedPitchSlider.setBounds (700, 660, 40, 50);
    matrix266SteppedFoldSlider.setBounds  (770, 660, 40, 50);
    matrix266SteppedLpgSlider.setBounds   (840, 660, 40, 50);
}
