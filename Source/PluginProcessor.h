#pragma once

#include <JuceHeader.h>

#include "dsp/WestPatchLane.h"
#include "dsp/NoiseSource.h"
#include "modulation/FunctionGenerator281.h"
#include "modulation/Uncertainty266.h"
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

enum class GroupMode
{
    Mono = 0,
    Duo,
    Quad
};

struct WestPatchGroup
{
    bool gate = false;
    int midiNote = -1;
    float frequency = 440.0f;
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
    bool funcBCycle = false;

    float uncertaintyRate = 1.2f;
    float uncertaintySmoothDepth = 0.8f;
    float uncertaintySteppedDepth = 8.0f;

    float synthLevel = 1.0f;
    float inputLevel = 0.0f;
    float noiseLevel = 0.0f;
    bool gateExternalInput = true;

    float test266SmoothToFold = 0.15f;
    float test266SteppedToPitch = 0.20f;
    float test266BiasToLpg = 0.10f;
    float test266PulseToTrigger = 0.0f;

    float detuneAmount = 0.35f;
    float stereoSpread = 0.60f;

    float complexModRatio = 2.0f;
    float complexFmAmount = 30.0f;
    float complexOscMix = 0.0f;

    GroupMode groupMode = GroupMode::Mono;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static constexpr int numModSources = (int) ModSource::Count;
    static constexpr int numModDestinations = (int) ModDestination::Count;

    float modulationMatrix[numModSources][numModDestinations] = {};

private:
    static constexpr int numLanes = 4;
    static constexpr int maxGroups = 4;

    WestPatchLane lanes[numLanes];
    WestPatchGroup groups[maxGroups];

    float laneDetuneBase[numLanes] = { -7.0f, -2.0f, 2.0f, 7.0f };
    float lanePanBase[numLanes]    = { -0.8f, -0.3f, 0.3f, 0.8f };

    NoiseSource noiseSource;
    FunctionGenerator281 functionGenerator281;
    Uncertainty266 uncertainty266;

    double currentSampleRate = 44100.0;
    float frequency = 440.0f;
    bool isNoteOn = false;
    int currentMidiNote = 69;
    float modEnvelope = 0.0f;
    float outputLevel = 0.15f;

    float smoothedFoldAmount = 2.5f;
    float smoothedAttackTime = 0.05f;
    float smoothedReleaseTime = 0.3f;
    float smoothedLpgAmount = 0.12f;
    float smoothedModDepth = 1.2f;
    float smoothedSynthLevel = 1.0f;
    float smoothedInputLevel = 0.0f;
    float smoothedNoiseLevel = 0.0f;

    float funcBPhase = 0.0f;
    float smoothedFuncBRate = 2.0f;
    float smoothedFuncBDepth = 20.0f;

    float uncertaintyPhase = 0.0f;
    float smoothRandomValue = 0.0f;
    float smoothRandomTarget = 0.0f;
    float steppedRandomValue = 0.0f;
    float smoothedUncertaintyRate = 1.2f;
    float smoothedUncertaintySmoothDepth = 0.8f;
    float smoothedUncertaintySteppedDepth = 8.0f;

    float smoothedTest266SmoothToFold = 0.15f;
    float smoothedTest266SteppedToPitch = 0.20f;
    float smoothedTest266BiasToLpg = 0.10f;
    float smoothedTest266PulseToTrigger = 0.0f;

    float smoothedDetuneAmount = 0.35f;
    float smoothedStereoSpread = 0.60f;

    float smoothedComplexModRatio = 2.0f;
    float smoothedComplexFmAmount = 30.0f;
    float smoothedComplexOscMix = 0.0f;

    bool previous266PulseHigh = false;

    bool hasActiveRoutingForDestination (ModDestination destination) const;
    int laneToGroup (int laneIndex) const noexcept;
    int getNumGroups() const noexcept;
    int findGroupForNoteOn() const noexcept;
    void noteOnToGroup (int midiNoteNumber) noexcept;
    void noteOffFromGroups (int midiNoteNumber) noexcept;
    void renderSample (float inputSample, float& outL, float& outR) noexcept;

    static float mapSteppedRandomToQuantizedSemitoneOffset (float steppedValue,
                                                            float depthNormalized) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WestPatchAudioProcessor)
};
