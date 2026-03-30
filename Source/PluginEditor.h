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

    juce::Image backgroundImage;

    // Top bar
    juce::Label groupModeLabel;
    juce::ComboBox groupModeBox;

    // Voice
    juce::Label attackLabel;
    juce::Slider attackSlider;
    juce::Label releaseLabel;
    juce::Slider releaseSlider;
    juce::Label foldLabel;
    juce::Slider foldSlider;
    juce::Label lpgLabel;
        juce::Slider lpgSlider;
        juce::ComboBox lpgModeBox;

    // 281
    juce::Label mod281AttackLabel;
    juce::Slider mod281AttackSlider;
    juce::Label mod281DecayLabel;
    juce::Slider mod281DecaySlider;
    juce::ComboBox func281ModeBox;
    juce::TextButton func281SyncButton { "Sync" };
        void updateSyncMode();
    juce::Label mod281PitchDepthLabel;
    juce::Slider mod281PitchDepthSlider;
    juce::Label mod281FoldDepthLabel;
    juce::Slider mod281FoldDepthSlider;
    juce::Label mod281LpgDepthLabel;
    juce::Slider mod281LpgDepthSlider;

    // 266
    juce::Label uncertaintyRateLabel;
    juce::Slider uncertaintyRateSlider;
    juce::Label uncertaintySmoothLabel;
    juce::Slider uncertaintySmoothSlider;
    juce::Label uncertaintySteppedLabel;
    juce::Slider uncertaintySteppedSlider;

    // Mixer
    juce::Label synthLevelLabel;
    juce::Slider synthLevelSlider;
    juce::Label inputLevelLabel;
    juce::Slider inputLevelSlider;
    juce::Label noiseLevelLabel;
    juce::Slider noiseLevelSlider;
    juce::ToggleButton gateInputButton;

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
        juce::Label oscShapeLabel;
        juce::Slider oscShapeSlider;

    // Matrix
    juce::Slider matrix281PitchSlider;
        juce::Slider matrix281FoldSlider;
        juce::Slider matrix281LpgSlider;
        juce::Slider matrix281Mod281AttackSlider;
        juce::Slider matrix281Mod281DecaySlider;
        juce::Slider matrix266SmoothPitchSlider;
        juce::Slider matrix266SmoothFoldSlider;
        juce::Slider matrix266SmoothLpgSlider;
        juce::Slider matrix266SmoothMod281AttackSlider;
        juce::Slider matrix266SmoothMod281DecaySlider;
        juce::Slider matrix266SteppedPitchSlider;
        juce::Slider matrix266SteppedFoldSlider;
        juce::Slider matrix266SteppedLpgSlider;
        juce::Slider matrix266SteppedMod281AttackSlider;
        juce::Slider matrix266SteppedMod281DecaySlider;
        juce::Slider func281SelfModSlider;
        juce::Label  func281SelfModLabel;
    
    void setupKnob (juce::Slider&, double min, double max, double interval);
    void setupDepthSlider (juce::Slider&);
    void setupMatrixSlider (juce::Slider&);
    void setupLabel (juce::Label&, const juce::String&,
                     juce::Justification = juce::Justification::centred);

    void drawModule (juce::Graphics&, juce::Rectangle<int> bounds,
                     const juce::String& title) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WestPatchAudioProcessorEditor)
};
