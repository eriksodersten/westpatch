#pragma once

#include <JuceHeader.h>

#include "core/GroupVoiceEngine.h"
#include "core/WestPatchGroupState.h"
#include "core/SignalBus.h"
#include "GroupEnvelopeManager.h"
#include "dsp/NoiseSource.h"
#include "dsp/WestPatchLane.h"
#include "modulation/FunctionGenerator281.h"
#include "modulation/Uncertainty266.h"

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

// GUI visar Unison / Duo / Quad
// Internt kan Mono stå kvar tills vidare.
enum class GroupMode
{
    Mono = 0,
    Duo,
    Quad
};

// Tone Mode är fortfarande globalt.
enum class ToneMode
{
    West = 0,
    Moog,
    Roland
};

//==============================================================================
class WestPatchAudioProcessor : public juce::AudioProcessor
{
public:
    WestPatchAudioProcessor();
    ~WestPatchAudioProcessor() override;

    //==============================================================================
    // AudioProcessor overrides
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #if ! JucePlugin_IsMidiEffect
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    // Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    // Basic plugin info
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Programs
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    // State
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Public params used by current editor

    // Voice envelope (group-level)
    float attackTime = 0.05f;
    float releaseTime = 0.30f;

    // Lane sound params
    float foldAmount = 2.5f;
    float lpgAmount = 0.12f;

    // Global modulation / 281
    float modAttackTime = 0.02f;
    float modReleaseTime = 0.25f;
    float modDepth = 2.5f;
    float funcBRate = 2.0f;
    float funcBDepth = 20.0f;
    bool funcBCycle = false;

    // Global 266
    float uncertaintyRate = 1.2f;
    float uncertaintySmoothDepth = 0.8f;
    float uncertaintySteppedDepth = 8.0f;

    // Mixer
    float synthLevel = 1.0f;
    float inputLevel = 0.0f;
    float noiseLevel = 0.0f;
    bool gateExternalInput = true;

    // Test / direct routing params kept global for now
    float test266SmoothToFold = 0.15f;
    float test266SteppedToPitch = 0.20f;
    float test266BiasToLpg = 0.10f;
    float test266PulseToTrigger = 0.0f;

    // Lane spread / complex oscillator
    float detuneAmount = 0.35f;
    float stereoSpread = 0.60f;
    float complexModRatio = 2.0f;
    float complexFmAmount = 30.0f;
    float complexOscMix = 0.0f;

    // Global modes
    GroupMode groupMode = GroupMode::Mono;
    ToneMode toneMode = ToneMode::West;

    void setGroupMode (GroupMode newMode) noexcept;
    int getActiveGroupCount() const noexcept;

    //==============================================================================
    // Mod matrix (still global)
    static constexpr int numModSources =
        static_cast<int> (ModSource::Count);

    static constexpr int numModDestinations =
        static_cast<int> (ModDestination::Count);

    float modulationMatrix[numModSources][numModDestinations] = {};

private:
    //==============================================================================
    static constexpr int numLanes = 4;
    static constexpr int maxGroups = 4;
    
    // Serial counters for group allocation ordering
    std::uint64_t groupAllocationSerial[maxGroups] = {};
    std::uint64_t nextAllocationSerial = 1;
    
    struct HandoffDebugState
    {
        bool active = false;
        int groupIndex = -1;
        int samplesRemaining = 0;
        int sampleCounter = 0;
        float oldFrequencyHz = 0.0f;
        float newFrequencyHz = 0.0f;
        bool oldGate = false;
        bool oldEnvActive = false;
    };

    static constexpr int handoffDebugSamples = 64;
    HandoffDebugState handoffDebug;
    int preHandoffDebugCount = 0;
    int fullBlockDebugCount = 0;
        std::array<float, 10> preHandoffRingBuffer {};
        int preHandoffRingIndex = 0;

        //==============================================================================
    // Crossfade state for handoff click suppression

    struct GroupCrossfadeState

    {

     bool active = false;

     int samplesRemaining = 0;
     float oldSignal[numLanes] = {};

    };

    static constexpr int crossfadeSamples = 64;

    GroupCrossfadeState crossfades[maxGroups];

    // Separate dying tail path for handoff.
    // Old lane DSP state dies here independently from the live group.

    static constexpr int tailReleaseSamples = 64;

    struct GroupTailState

    {

     bool active = false;

     int samplesRemaining = 0;
     float frequencyHz = 440.0f;
     float gain = 0.0f;
     float gainStep = 0.0f;
     float pitchModSemitones = 0.0f;
     float foldModAmount = 0.0f;
     float lpgCvMod = 0.0f;
     WestPatchLane lanes[numLanes];

    };

    GroupTailState tails[maxGroups];

    float laneOutputCache[numLanes] = {};
    float lastRenderedGroupEnv[maxGroups] = {};
    
    
    //==============================================================================
    // Always 4 lanes
    WestPatchLane lanes[numLanes];

    westpatch::engine::GroupVoiceEngine newEngine;

    // 1/2/4 logical groups depending on mode
    WestPatchGroupState groups[maxGroups];
    GroupEnvelopeManager groupEnvelopeManager;

    //==============================================================================
    // Still global
    NoiseSource noiseSource;
    FunctionGenerator281 functionGenerator281;
    Uncertainty266 uncertainty266;

    //==============================================================================
    // Runtime / cached state
    double currentSampleRate = 44100.0;
    float outputLevel = 0.15f;

    float laneDetuneBase[numLanes] = { -7.0f, -2.0f, 2.0f, 7.0f };
    float lanePanBase[numLanes]    = { -0.8f, -0.3f, 0.3f, 0.8f };

    bool previous266PulseHigh = false;

    //==============================================================================
    // Helpers
    int findGroupForNoteOff (int midiNoteNumber) const noexcept;
    int laneToGroup (int laneIndex) const noexcept;
    int getNumGroups() const noexcept;
    int findGroupForNoteOn() const noexcept;

    void resetGroups() noexcept;
    void noteOnToGroup (int midiNoteNumber) noexcept;
    void noteOffFromGroups (int midiNoteNumber) noexcept;

    void renderSample (float inputSample, float& outL, float& outR) noexcept;

    bool hasActiveRoutingForDestination (ModDestination destination) const;
    static float mapSteppedRandomToQuantizedSemitoneOffset (float steppedValue,
                                                            float depthNormalized) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WestPatchAudioProcessor)
};
