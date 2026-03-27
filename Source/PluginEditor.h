#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class WestPatchAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit WestPatchAudioProcessorEditor (WestPatchAudioProcessor&);
    ~WestPatchAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    WestPatchAudioProcessor& audioProcessor;

    juce::Image backgroundImage;

    //==========================================================================
    // Group
    juce::Label groupModeLabel;
    juce::ComboBox groupModeBox;

    //==========================================================================
    // Voice
    juce::Label attackLabel;
    juce::Slider attackSlider;

    juce::Label releaseLabel;
    juce::Slider releaseSlider;

    juce::Label foldLabel;
    juce::Slider foldSlider;

    juce::Label lpgLabel;
    juce::Slider lpgSlider;

    //==========================================================================
    // 281
    juce::Label funcBRateLabel;
    juce::Slider funcBRateSlider;

    juce::Label funcBDepthLabel;
    juce::Slider funcBDepthSlider;

    juce::ComboBox func281ModeBox;

    //==========================================================================
    // 266
    juce::Label uncertaintyRateLabel;
    juce::Slider uncertaintyRateSlider;

    juce::Label uncertaintySmoothLabel;
    juce::Slider uncertaintySmoothSlider;

    juce::Label uncertaintySteppedLabel;
    juce::Slider uncertaintySteppedSlider;

    //==========================================================================
    // Mixer
    juce::Label synthLevelLabel;
    juce::Slider synthLevelSlider;

    juce::Label inputLevelLabel;
    juce::Slider inputLevelSlider;

    juce::Label noiseLevelLabel;
    juce::Slider noiseLevelSlider;

    juce::ToggleButton gateInputButton;

    //==========================================================================
    // 266 direct routing / tests
    juce::Label test266SmoothToFoldLabel;
    juce::Slider test266SmoothToFoldSlider;

    juce::Label test266SteppedToPitchLabel;
    juce::Slider test266SteppedToPitchSlider;

    juce::Label test266BiasToLpgLabel;
    juce::Slider test266BiasToLpgSlider;

    juce::Label test266PulseToTriggerLabel;
    juce::Slider test266PulseToTriggerSlider;

    //==========================================================================
    // Lane / complex
    juce::Label detuneAmountLabel;
    juce::Slider detuneAmountSlider;

    juce::Label stereoSpreadLabel;
    juce::Slider stereoSpreadSlider;

    juce::Label complexModRatioLabel;
    juce::Slider complexModRatioSlider;

    juce::Label complexFmAmountLabel;
    juce::Slider complexFmAmountSlider;

    juce::Label complexOscMixLabel;
    juce::Slider complexOscMixSlider;

    //==========================================================================
    // Matrix
    juce::Label matrixTitleLabel;

    juce::Label matrixSource281Label;
    juce::Label matrixSource266SmoothLabel;
    juce::Label matrixSource266SteppedLabel;

    juce::Label matrixPitchLabel;
    juce::Label matrixFoldLabel;
    juce::Label matrixLpgLabel;

    juce::Slider matrix281PitchSlider;
    juce::Slider matrix281FoldSlider;
    juce::Slider matrix281LpgSlider;

    juce::Slider matrix266SmoothPitchSlider;
    juce::Slider matrix266SmoothFoldSlider;
    juce::Slider matrix266SmoothLpgSlider;

    juce::Slider matrix266SteppedPitchSlider;
    juce::Slider matrix266SteppedFoldSlider;
    juce::Slider matrix266SteppedLpgSlider;

    //==========================================================================
    void setupKnob (juce::Slider& slider,
                    double min,
                    double max,
                    double interval);

    void setupMatrixSlider (juce::Slider& slider);

    void setupLabel (juce::Label& label,
                     const juce::String& text,
                     juce::Justification justification = juce::Justification::centred);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WestPatchAudioProcessorEditor)
};
