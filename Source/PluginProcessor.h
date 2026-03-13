#pragma once

#include <JuceHeader.h>

#include "dsp/Oscillator.h"
#include "dsp/NoiseSource.h"
#include "dsp/Wavefolder.h"
#include "dsp/LowPassGate.h"
#include "modulation/FunctionGenerator281.h"
#include "core/SignalBus.h"

//==============================================================================
enum class ModSource
{
    FunctionB = 0,
    UncertaintySmooth,
    UncertaintyStepped,
    Count
};

enum class ModDestination
{
    Pitch = 0,
    Fold,
    LPG,
    Count
};

//==============================================================================
class WestPatchAudioProcessor : public juce::AudioProcessor
{
public:
    WestPatchAudioProcessor();
    ~WestPatchAudioProcessor() override;

    float attackTime = 0.05f;
    float releaseTime = 0.3f;

    float foldAmount = 2.5f;
    float lpgAmount = 0.12f;

    float modAttackTime = 0.02f;
    float modReleaseTime = 0.25f;
    float modDepth = 2.5f;

    float funcBRate = 2.0f;
    float funcBDepth = 20.0f;
    bool funcBCycle = true;

    float uncertaintyRate = 1.2f;
    float uncertaintySmoothDepth = 0.8f;
    float uncertaintySteppedDepth = 8.0f;

    float synthLevel = 1.0f;
    float inputLevel = 0.0f;
    float noiseLevel = 0.0f;

    bool gateExternalInput = true;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    static constexpr int numModSources = (int) ModSource::Count;
    static constexpr int numModDestinations = (int) ModDestination::Count;

    float modulationMatrix[numModSources][numModDestinations] = {};

private:
    // DSP modules
    Oscillator oscillator;
    NoiseSource noiseSource;
    Wavefolder wavefolder;
    LowPassGate lowPassGate;
    FunctionGenerator281 functionGenerator281;

    // Engine state
    double currentSampleRate = 44100.0;

    float frequency = 440.0f;
    bool isNoteOn = false;
    int currentMidiNote = 69;

    float modEnvelope = 0.0f;
    float outputLevel = 0.15f;

    // Smoothed parameters
    float smoothedFoldAmount = 2.5f;
    float smoothedAttackTime = 0.05f;
    float smoothedReleaseTime = 0.3f;
    float smoothedLpgAmount = 0.12f;
    float smoothedModDepth = 1.2f;

    float smoothedSynthLevel = 1.0f;
    float smoothedInputLevel = 0.0f;
    float smoothedNoiseLevel = 0.0f;

    // Function generator B
    float funcBPhase = 0.0f;
    float smoothedFuncBRate = 2.0f;
    float smoothedFuncBDepth = 20.0f;

    // Uncertainty
    float uncertaintyPhase = 0.0f;
    float smoothRandomValue = 0.0f;
    float smoothRandomTarget = 0.0f;
    float steppedRandomValue = 0.0f;

    float smoothedUncertaintyRate = 1.2f;
    float smoothedUncertaintySmoothDepth = 0.8f;
    float smoothedUncertaintySteppedDepth = 8.0f;

    bool hasActiveRoutingForDestination (ModDestination destination) const;
    float renderSample (float inputSample) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WestPatchAudioProcessor)
};
