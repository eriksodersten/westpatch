#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class WestPatchAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit WestPatchAudioProcessorEditor (WestPatchAudioProcessor&);
    ~WestPatchAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    WestPatchAudioProcessor& audioProcessor;

    juce::Slider foldSlider;
    juce::Slider attackSlider;
    juce::Slider releaseSlider;
    juce::Slider lpgSlider;

    juce::Slider funcBRateSlider;
    juce::Slider funcBDepthSlider;

    juce::Slider uncertaintyRateSlider;
    juce::Slider uncertaintySmoothSlider;
    juce::Slider uncertaintySteppedSlider;

    juce::Slider synthLevelSlider;
    juce::Slider inputLevelSlider;
    juce::Slider noiseLevelSlider;

    juce::Slider test266SmoothToFoldSlider;
    juce::Slider test266SteppedToPitchSlider;
    juce::Slider test266BiasToLpgSlider;
    juce::Slider test266PulseToTriggerSlider;

    juce::Slider detuneAmountSlider;
    juce::Slider stereoSpreadSlider;

    juce::Slider complexModRatioSlider;
    juce::Slider complexFmAmountSlider;
    juce::Slider complexOscMixSlider;

    juce::Slider matrix281PitchSlider;
    juce::Slider matrix281FoldSlider;
    juce::Slider matrix281LpgSlider;

    juce::Slider matrix266SmoothPitchSlider;
    juce::Slider matrix266SmoothFoldSlider;
    juce::Slider matrix266SmoothLpgSlider;

    juce::Slider matrix266SteppedPitchSlider;
    juce::Slider matrix266SteppedFoldSlider;
    juce::Slider matrix266SteppedLpgSlider;

    juce::Label foldLabel;
    juce::Label attackLabel;
    juce::Label releaseLabel;
    juce::Label lpgLabel;

    juce::Label funcBRateLabel;
    juce::Label funcBDepthLabel;

    juce::Label uncertaintyRateLabel;
    juce::Label uncertaintySmoothLabel;
    juce::Label uncertaintySteppedLabel;

    juce::Label synthLevelLabel;
    juce::Label inputLevelLabel;
    juce::Label noiseLevelLabel;

    juce::Label test266SmoothToFoldLabel;
    juce::Label test266SteppedToPitchLabel;
    juce::Label test266BiasToLpgLabel;
    juce::Label test266PulseToTriggerLabel;

    juce::Label detuneAmountLabel;
    juce::Label stereoSpreadLabel;

    juce::Label complexModRatioLabel;
    juce::Label complexFmAmountLabel;
    juce::Label complexOscMixLabel;

    juce::Label matrixTitleLabel;
    juce::Label matrixPitchLabel;
    juce::Label matrixFoldLabel;
    juce::Label matrixLpgLabel;
    juce::Label matrix281Label;
    juce::Label matrix266SmoothLabel;
    juce::Label matrix266SteppedLabel;

    juce::ToggleButton funcBCycleButton;
    juce::ToggleButton gateInputButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WestPatchAudioProcessorEditor)
};
