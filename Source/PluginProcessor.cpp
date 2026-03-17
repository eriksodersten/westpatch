#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================================
WestPatchAudioProcessor::WestPatchAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
 #if ! JucePlugin_IsSynth
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
 #endif
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
{
    for (int s = 0; s < numModSources; ++s)
        for (int d = 0; d < numModDestinations; ++d)
            modulationMatrix[s][d] = 0.0f;
}

WestPatchAudioProcessor::~WestPatchAudioProcessor() = default;

//==============================================================================
const juce::String WestPatchAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WestPatchAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WestPatchAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WestPatchAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WestPatchAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WestPatchAudioProcessor::getNumPrograms()
{
    return 1;
}

int WestPatchAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WestPatchAudioProcessor::setCurrentProgram (int)
{
}

const juce::String WestPatchAudioProcessor::getProgramName (int)
{
    return {};
}

void WestPatchAudioProcessor::changeProgramName (int, const juce::String&)
{
}

//==============================================================================
void WestPatchAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;

    for (int i = 0; i < numLanes; ++i)
    {
        lanes[i].prepare (sampleRate);
        lanes[i].frequency = frequency;
    }

    for (int g = 0; g < maxGroups; ++g)
    {
        groups[g].gate = false;
        groups[g].midiNote = -1;
        groups[g].frequency = frequency;
    }

    noiseSource.prepare (sampleRate);
    functionGenerator281.prepare (sampleRate);
    functionGenerator281.setCycle (funcBCycle);
    uncertainty266.prepare (sampleRate);

    smoothedFoldAmount = foldAmount;
    smoothedAttackTime = attackTime;
    smoothedReleaseTime = releaseTime;
    smoothedLpgAmount = lpgAmount;
    smoothedModDepth = modDepth;
    smoothedFuncBRate = funcBRate;
    smoothedFuncBDepth = funcBDepth;
    smoothedUncertaintyRate = uncertaintyRate;
    smoothedUncertaintySmoothDepth = uncertaintySmoothDepth;
    smoothedUncertaintySteppedDepth = uncertaintySteppedDepth;
    smoothedSynthLevel = synthLevel;
    smoothedInputLevel = inputLevel;
    smoothedNoiseLevel = noiseLevel;

    smoothedTest266SmoothToFold = test266SmoothToFold;
    smoothedTest266SteppedToPitch = test266SteppedToPitch;
    smoothedTest266BiasToLpg = test266BiasToLpg;
    smoothedTest266PulseToTrigger = test266PulseToTrigger;

    smoothedDetuneAmount = detuneAmount;
    smoothedStereoSpread = stereoSpread;

    smoothedComplexModRatio = complexModRatio;
    smoothedComplexFmAmount = complexFmAmount;
    smoothedComplexOscMix = complexOscMix;

    modEnvelope = 0.0f;
    funcBPhase = 0.0f;
    uncertaintyPhase = 0.0f;
    smoothRandomValue = 0.0f;
    smoothRandomTarget = 0.0f;
    steppedRandomValue = 0.0f;
    previous266PulseHigh = false;
}

void WestPatchAudioProcessor::releaseResources()
{
}

//==============================================================================
bool WestPatchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
#endif
}

bool WestPatchAudioProcessor::hasActiveRoutingForDestination (ModDestination destination) const
{
    const int d = static_cast<int> (destination);

    for (int s = 0; s < numModSources; ++s)
        if (modulationMatrix[s][d] != 0.0f)
            return true;

    return false;
}

int WestPatchAudioProcessor::laneToGroup (int laneIndex) const noexcept
{
    switch (groupMode)
    {
        case GroupMode::Mono:
            return 0;

        case GroupMode::Duo:
            return (laneIndex < 2) ? 0 : 1;

        case GroupMode::Quad:
            return juce::jlimit (0, maxGroups - 1, laneIndex);
    }

    return 0;
}

int WestPatchAudioProcessor::getNumGroups() const noexcept
{
    switch (groupMode)
    {
        case GroupMode::Mono: return 1;
        case GroupMode::Duo:  return 2;
        case GroupMode::Quad: return 4;
    }

    return 1;
}

int WestPatchAudioProcessor::findGroupForNoteOn() const noexcept
{
    const int numGroups = getNumGroups();

    // First free group
    for (int g = 0; g < numGroups; ++g)
        if (! groups[g].gate)
            return g;

    // Fallback: steal the last active group
    return juce::jmax (0, numGroups - 1);
}

void WestPatchAudioProcessor::noteOnToGroup (int midiNoteNumber) noexcept
{
    const int groupIndex = findGroupForNoteOn();

    groups[groupIndex].gate = true;
    groups[groupIndex].midiNote = midiNoteNumber;
    groups[groupIndex].frequency =
        juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);

    currentMidiNote = midiNoteNumber;
    frequency = groups[groupIndex].frequency;
    isNoteOn = true;
}

void WestPatchAudioProcessor::noteOffFromGroups (int midiNoteNumber) noexcept
{
    const int numGroups = getNumGroups();

    for (int g = 0; g < numGroups; ++g)
    {
        if (groups[g].midiNote == midiNoteNumber)
        {
            groups[g].gate = false;
            groups[g].midiNote = -1;
        }
    }

    bool anyGate = false;
    for (int g = 0; g < numGroups; ++g)
    {
        if (groups[g].gate)
        {
            anyGate = true;
            currentMidiNote = groups[g].midiNote;
            frequency = groups[g].frequency;
            break;
        }
    }

    isNoteOn = anyGate;
}

float WestPatchAudioProcessor::mapSteppedRandomToQuantizedSemitoneOffset (float steppedValue,
                                                                          float depthNormalized) noexcept
{
    static constexpr int scaleOffsets[] =
    {
        -12, -10, -9, -7, -5, -3, -2,
          0,
          2,   3,  5,  7,  9, 10, 12
    };

    static constexpr int numOffsets = (int) (sizeof (scaleOffsets) / sizeof (scaleOffsets[0]));

    const float clampedDepth = juce::jlimit (0.0f, 1.0f, depthNormalized);
    if (clampedDepth <= 0.0001f)
        return 0.0f;

    const float clampedStepped = juce::jlimit (-1.0f, 1.0f, steppedValue);
    const float normalized = 0.5f * (clampedStepped + 1.0f);

    const int centerIndex = numOffsets / 2;
    const int maxSpread = centerIndex;

    const int allowedSpread = juce::jlimit (0,
                                            maxSpread,
                                            (int) std::round (clampedDepth * (float) maxSpread));

    if (allowedSpread == 0)
        return 0.0f;

    const int minIndex = centerIndex - allowedSpread;
    const int maxIndex = centerIndex + allowedSpread;

    const float mappedIndex = juce::jmap (normalized, (float) minIndex, (float) maxIndex);
    const int index = juce::jlimit (minIndex, maxIndex, (int) std::round (mappedIndex));

    return (float) scaleOffsets[index];
}

//==============================================================================
void WestPatchAudioProcessor::renderSample (float inputSample, float& outL, float& outR) noexcept
{
    constexpr float smoothingCoeff = 0.001f;
    constexpr float foldTestGain = 5.0f;

    SignalBus bus;

    smoothedFoldAmount += smoothingCoeff * (foldAmount - smoothedFoldAmount);
    smoothedAttackTime += smoothingCoeff * (attackTime - smoothedAttackTime);
    smoothedReleaseTime += smoothingCoeff * (releaseTime - smoothedReleaseTime);
    smoothedLpgAmount += smoothingCoeff * (lpgAmount - smoothedLpgAmount);
    smoothedModDepth += smoothingCoeff * (modDepth - smoothedModDepth);
    smoothedFuncBRate += smoothingCoeff * (funcBRate - smoothedFuncBRate);
    smoothedFuncBDepth += smoothingCoeff * (funcBDepth - smoothedFuncBDepth);
    smoothedUncertaintyRate += smoothingCoeff * (uncertaintyRate - smoothedUncertaintyRate);
    smoothedUncertaintySmoothDepth += smoothingCoeff * (uncertaintySmoothDepth - smoothedUncertaintySmoothDepth);
    smoothedUncertaintySteppedDepth += smoothingCoeff * (uncertaintySteppedDepth - smoothedUncertaintySteppedDepth);
    smoothedSynthLevel += smoothingCoeff * (synthLevel - smoothedSynthLevel);
    smoothedInputLevel += smoothingCoeff * (inputLevel - smoothedInputLevel);
    smoothedNoiseLevel += smoothingCoeff * (noiseLevel - smoothedNoiseLevel);

    smoothedTest266SmoothToFold += smoothingCoeff * (test266SmoothToFold - smoothedTest266SmoothToFold);
    smoothedTest266SteppedToPitch += smoothingCoeff * (test266SteppedToPitch - smoothedTest266SteppedToPitch);
    smoothedTest266BiasToLpg += smoothingCoeff * (test266BiasToLpg - smoothedTest266BiasToLpg);
    smoothedTest266PulseToTrigger += smoothingCoeff * (test266PulseToTrigger - smoothedTest266PulseToTrigger);
    smoothedDetuneAmount += smoothingCoeff * (detuneAmount - smoothedDetuneAmount);
    smoothedStereoSpread += smoothingCoeff * (stereoSpread - smoothedStereoSpread);
    smoothedComplexModRatio += smoothingCoeff * (complexModRatio - smoothedComplexModRatio);
    smoothedComplexFmAmount += smoothingCoeff * (complexFmAmount - smoothedComplexFmAmount);
    smoothedComplexOscMix += smoothingCoeff * (complexOscMix - smoothedComplexOscMix);

    bus.env = functionGenerator281.process (smoothedAttackTime, smoothedReleaseTime);

    const float modEnvCoeff = isNoteOn
        ? (1.0f / juce::jmax (1.0f, modAttackTime * static_cast<float> (currentSampleRate)))
        : (1.0f / juce::jmax (1.0f, modReleaseTime * static_cast<float> (currentSampleRate)));

    if (isNoteOn)
        modEnvelope += (1.0f - modEnvelope) * modEnvCoeff;
    else
        modEnvelope += (0.0f - modEnvelope) * modEnvCoeff;

    bus.modEnv = modEnvelope;

    funcBPhase += smoothedFuncBRate / static_cast<float> (currentSampleRate);
    if (funcBPhase >= 1.0f)
        funcBPhase -= 1.0f;

    const float functionBValue =
        std::sin (2.0f * juce::MathConstants<float>::pi * funcBPhase) * smoothedFuncBDepth;

    Uncertainty266::Params uncertaintyParams;
    uncertaintyParams.rate = smoothedUncertaintyRate;
    uncertaintyParams.smoothAmount = smoothedUncertaintySmoothDepth;
    uncertaintyParams.steppedAmount = smoothedUncertaintySteppedDepth;
    uncertaintyParams.density = 0.75f;
    uncertaintyParams.correlation = 0.45f;
    uncertaintyParams.spread = 1.0f;

    const auto uncertaintyOut = uncertainty266.process (uncertaintyParams);

    const float uncertaintySmoothValue  = uncertaintyOut.smooth;
    const float uncertaintySteppedValue = uncertaintyOut.stepped;
    const float uncertaintyBiasValue    = uncertaintyOut.bias;
    const float uncertaintyPulseValue   = uncertaintyOut.pulse;

    bus.pitchMod = 0.0f;
    bus.foldMod = 0.0f;
    bus.lpgCV = 0.0f;

    bus.pitchMod += functionBValue
        * modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Pitch];
    bus.foldMod += functionBValue
        * modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Fold];
    bus.lpgCV += functionBValue
        * modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::LPG];

    bus.pitchMod += uncertaintySmoothValue
        * modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Pitch];
    bus.foldMod += uncertaintySmoothValue
        * modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Fold];
    bus.lpgCV += uncertaintySmoothValue
        * modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::LPG];

    bus.pitchMod += uncertaintySteppedValue
        * modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Pitch];
    bus.foldMod += uncertaintySteppedValue
        * modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Fold];
    bus.lpgCV += uncertaintySteppedValue
        * modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::LPG];

    const float testFoldMod =
        (smoothedTest266SmoothToFold <= 0.0001f)
            ? 0.0f
            : uncertaintySmoothValue * smoothedTest266SmoothToFold * foldTestGain;

    const float testLpgMod =
        (smoothedTest266BiasToLpg <= 0.0001f)
            ? 0.0f
            : uncertaintyBiasValue * smoothedTest266BiasToLpg;

    const float quantizedSemitoneOffset =
        mapSteppedRandomToQuantizedSemitoneOffset (uncertaintySteppedValue,
                                                   smoothedTest266SteppedToPitch);

    const float pitchRatio =
        (std::abs (quantizedSemitoneOffset) <= 0.0001f)
            ? 1.0f
            : std::pow (2.0f, quantizedSemitoneOffset / 12.0f);

    const bool pulseHigh = uncertaintyPulseValue > 0.5f;
    if (smoothedTest266PulseToTrigger > 0.01f && pulseHigh && ! previous266PulseHigh)
        functionGenerator281.trigger();

    previous266PulseHigh = pulseHigh;

    bus.noise = noiseSource.process();
    bus.extIn = inputSample;

    bus.synthIn = smoothedSynthLevel * 1.15f;
    bus.noiseIn = bus.noise * smoothedNoiseLevel;
    bus.extScaled = bus.extIn * smoothedInputLevel;

    const float foldValue = smoothedFoldAmount + bus.foldMod + testFoldMod;
    const float lpgEnvelope = bus.env * (1.5f - 0.5f * bus.env);
    const float ampEnvelope = bus.env;

    float laneMixL = 0.0f;
    float laneMixR = 0.0f;

    for (int i = 0; i < numLanes; ++i)
    {
        auto& lane = lanes[i];
        const int groupIndex = laneToGroup (i);
        const float groupFrequency = groups[groupIndex].frequency;

        const float detuneCents = laneDetuneBase[i] * smoothedDetuneAmount;
        const float detuneRatio = std::pow (2.0f, detuneCents / 1200.0f);

        const float modulatedFrequencyHz =
            juce::jmax (20.0f, (groupFrequency + bus.pitchMod) * pitchRatio);

        const float laneOut = lane.renderComplex (
            modulatedFrequencyHz * detuneRatio,
            smoothedComplexModRatio,
            smoothedComplexFmAmount,
            smoothedComplexOscMix,
            bus.synthIn,
            foldValue,
            lpgEnvelope,
            smoothedLpgAmount,
            bus.lpgCV + testLpgMod,
            bus.noiseIn
        );

        const float pan = juce::jlimit (-1.0f, 1.0f, lanePanBase[i] * smoothedStereoSpread);
        const float gainL = 0.5f * (1.0f - pan);
        const float gainR = 0.5f * (1.0f + pan);

        laneMixL += laneOut * gainL;
        laneMixR += laneOut * gainR;
    }

    laneMixL *= (1.0f / (float) numLanes);
    laneMixR *= (1.0f / (float) numLanes);

    const float extFolded = lanes[0].wavefolder.process (bus.extScaled, foldValue);

    if (gateExternalInput)
        bus.extOut = extFolded * ampEnvelope;
    else
        bus.extOut = extFolded;

    outL = (laneMixL + bus.extOut) * outputLevel;
    outR = (laneMixR + bus.extOut) * outputLevel;
}

//==============================================================================
void WestPatchAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    functionGenerator281.setCycle (funcBCycle);

    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            noteOnToGroup (message.getNoteNumber());
            functionGenerator281.trigger();
        }
        else if (message.isNoteOff())
        {
            noteOffFromGroups (message.getNoteNumber());
        }
    }

    auto* leftOut  = buffer.getWritePointer (0);
    auto* rightOut = totalNumOutputChannels > 1 ? buffer.getWritePointer (1) : nullptr;
    const float* inputLeft = totalNumInputChannels > 0 ? buffer.getReadPointer (0) : nullptr;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float inputSample = inputLeft != nullptr ? inputLeft[sample] : 0.0f;

        float outL = 0.0f;
        float outR = 0.0f;
        renderSample (inputSample, outL, outR);

        leftOut[sample] = outL;

        if (rightOut != nullptr)
            rightOut[sample] = outR;
    }
}

//==============================================================================
bool WestPatchAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* WestPatchAudioProcessor::createEditor()
{
    return new WestPatchAudioProcessorEditor (*this);
}

//==============================================================================
void WestPatchAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream (destData, false);

    stream.writeFloat (attackTime);
    stream.writeFloat (releaseTime);
    stream.writeFloat (foldAmount);
    stream.writeFloat (lpgAmount);
    stream.writeFloat (modAttackTime);
    stream.writeFloat (modReleaseTime);
    stream.writeFloat (modDepth);
    stream.writeFloat (funcBRate);
    stream.writeFloat (funcBDepth);
    stream.writeBool  (funcBCycle);
    stream.writeFloat (uncertaintyRate);
    stream.writeFloat (uncertaintySmoothDepth);
    stream.writeFloat (uncertaintySteppedDepth);
    stream.writeFloat (synthLevel);
    stream.writeFloat (inputLevel);
    stream.writeFloat (noiseLevel);
    stream.writeBool  (gateExternalInput);

    stream.writeFloat (test266SmoothToFold);
    stream.writeFloat (test266SteppedToPitch);
    stream.writeFloat (test266BiasToLpg);
    stream.writeFloat (test266PulseToTrigger);

    stream.writeFloat (detuneAmount);
    stream.writeFloat (stereoSpread);

    stream.writeFloat (complexModRatio);
    stream.writeFloat (complexFmAmount);
    stream.writeFloat (complexOscMix);

    stream.writeInt ((int) groupMode);

    for (int s = 0; s < numModSources; ++s)
        for (int d = 0; d < numModDestinations; ++d)
            stream.writeFloat (modulationMatrix[s][d]);
}

void WestPatchAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);

    auto readFloatOr = [&stream] (float fallbackValue) -> float
    {
        return stream.getNumBytesRemaining() >= (int) sizeof (float) ? stream.readFloat() : fallbackValue;
    };

    auto readBoolOr = [&stream] (bool fallbackValue) -> bool
    {
        return stream.getNumBytesRemaining() >= (int) sizeof (bool) ? stream.readBool() : fallbackValue;
    };

    auto readIntOr = [&stream] (int fallbackValue) -> int
    {
        return stream.getNumBytesRemaining() >= (int) sizeof (int) ? stream.readInt() : fallbackValue;
    };

    attackTime = readFloatOr (attackTime);
    releaseTime = readFloatOr (releaseTime);
    foldAmount = readFloatOr (foldAmount);
    lpgAmount = readFloatOr (lpgAmount);
    modAttackTime = readFloatOr (modAttackTime);
    modReleaseTime = readFloatOr (modReleaseTime);
    modDepth = readFloatOr (modDepth);
    funcBRate = readFloatOr (funcBRate);
    funcBDepth = readFloatOr (funcBDepth);
    funcBCycle = readBoolOr (funcBCycle);
    uncertaintyRate = readFloatOr (uncertaintyRate);
    uncertaintySmoothDepth = readFloatOr (uncertaintySmoothDepth);
    uncertaintySteppedDepth = readFloatOr (uncertaintySteppedDepth);
    synthLevel = readFloatOr (synthLevel);
    inputLevel = readFloatOr (inputLevel);
    noiseLevel = readFloatOr (noiseLevel);
    gateExternalInput = readBoolOr (gateExternalInput);

    test266SmoothToFold = readFloatOr (test266SmoothToFold);
    test266SteppedToPitch = readFloatOr (test266SteppedToPitch);
    test266BiasToLpg = readFloatOr (test266BiasToLpg);
    test266PulseToTrigger = readFloatOr (test266PulseToTrigger);

    detuneAmount = readFloatOr (detuneAmount);
    stereoSpread = readFloatOr (stereoSpread);

    complexModRatio = readFloatOr (complexModRatio);
    complexFmAmount = readFloatOr (complexFmAmount);
    complexOscMix = readFloatOr (complexOscMix);

    groupMode = (GroupMode) juce::jlimit (0, 2, readIntOr ((int) groupMode));

    for (int s = 0; s < numModSources; ++s)
        for (int d = 0; d < numModDestinations; ++d)
            modulationMatrix[s][d] = readFloatOr (modulationMatrix[s][d]);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WestPatchAudioProcessor();
}
