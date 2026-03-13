#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================================
WestPatchAudioProcessor::WestPatchAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
 #if ! JucePlugin_IsSynth
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
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

    oscillator.prepare (sampleRate);
    noiseSource.prepare (sampleRate);
    lowPassGate.prepare (sampleRate);
    functionGenerator281.prepare (sampleRate);
    functionGenerator281.setCycle (funcBCycle);

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

    modEnvelope = 0.0f;

    funcBPhase = 0.0f;
    uncertaintyPhase = 0.0f;
    smoothRandomValue = 0.0f;
    smoothRandomTarget = 0.0f;
    steppedRandomValue = 0.0f;
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

//==============================================================================
float WestPatchAudioProcessor::renderSample (float inputSample) noexcept
{
    constexpr float smoothingCoeff = 0.001f;

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

    bus.env = functionGenerator281.process (
        smoothedAttackTime,
        smoothedReleaseTime
    );

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

    uncertaintyPhase += smoothedUncertaintyRate / static_cast<float> (currentSampleRate);
    if (uncertaintyPhase >= 1.0f)
    {
        uncertaintyPhase -= 1.0f;
        smoothRandomTarget = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
        steppedRandomValue = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
    }

    smoothRandomValue += 0.001f * (smoothRandomTarget - smoothRandomValue);

    const float uncertaintySmoothValue =
        smoothRandomValue * smoothedUncertaintySmoothDepth;

    const float uncertaintySteppedValue =
        steppedRandomValue * smoothedUncertaintySteppedDepth;

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

    const float modulatedFrequency = juce::jmax (20.0f, frequency + bus.pitchMod);

    bus.osc = oscillator.process (modulatedFrequency);
    bus.noise = noiseSource.process();
    bus.extIn = inputSample;

    bus.synthIn = bus.osc * smoothedSynthLevel;
    bus.noiseIn = bus.noise * smoothedNoiseLevel;
    bus.extScaled = bus.extIn * smoothedInputLevel;

    const float foldValue = smoothedFoldAmount + bus.foldMod;

    bus.synthFolded = wavefolder.process (bus.synthIn, foldValue);
    bus.noiseFolded = wavefolder.process (bus.noiseIn, foldValue);
    bus.extFolded   = wavefolder.process (bus.extScaled, foldValue);

    bus.synthBus = bus.synthFolded + bus.noiseFolded;

    const float lpgEnvelope = bus.env * bus.env;
    const float ampEnvelope = std::sqrt (juce::jmax (0.0f, bus.env));

    bus.lpgOut = lowPassGate.process (
        bus.synthBus,
        lpgEnvelope,
        smoothedLpgAmount,
        bus.lpgCV
    );

    if (gateExternalInput)
        bus.extOut = bus.extFolded * ampEnvelope;
    else
        bus.extOut = bus.extFolded;

    bus.output = (bus.lpgOut + bus.extOut) * outputLevel;

    return bus.output;
}

//==============================================================================
void WestPatchAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
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
            currentMidiNote = message.getNoteNumber();
            frequency = juce::MidiMessage::getMidiNoteInHertz (currentMidiNote);
            isNoteOn = true;
            functionGenerator281.trigger();
        }
        else if (message.isNoteOff())
        {
            if (message.getNoteNumber() == currentMidiNote)
                isNoteOn = false;
        }
    }

    auto* leftOut  = buffer.getWritePointer (0);
    auto* rightOut = totalNumOutputChannels > 1 ? buffer.getWritePointer (1) : nullptr;

    const float* inputLeft = totalNumInputChannels > 0 ? buffer.getReadPointer (0) : nullptr;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const float inputSample = inputLeft != nullptr ? inputLeft[sample] : 0.0f;
        const float value = renderSample (inputSample);

        leftOut[sample] = value;
        if (rightOut != nullptr)
            rightOut[sample] = value;
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
    stream.writeBool (funcBCycle);

    stream.writeFloat (uncertaintyRate);
    stream.writeFloat (uncertaintySmoothDepth);
    stream.writeFloat (uncertaintySteppedDepth);

    stream.writeFloat (synthLevel);
    stream.writeFloat (inputLevel);
    stream.writeFloat (noiseLevel);
    stream.writeBool (gateExternalInput);

    for (int s = 0; s < numModSources; ++s)
        for (int d = 0; d < numModDestinations; ++d)
            stream.writeFloat (modulationMatrix[s][d]);
}

void WestPatchAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);

    attackTime = stream.readFloat();
    releaseTime = stream.readFloat();
    foldAmount = stream.readFloat();
    lpgAmount = stream.readFloat();

    modAttackTime = stream.readFloat();
    modReleaseTime = stream.readFloat();
    modDepth = stream.readFloat();

    funcBRate = stream.readFloat();
    funcBDepth = stream.readFloat();
    funcBCycle = stream.readBool();

    uncertaintyRate = stream.readFloat();
    uncertaintySmoothDepth = stream.readFloat();
    uncertaintySteppedDepth = stream.readFloat();

    synthLevel = stream.readFloat();
    inputLevel = stream.readFloat();
    noiseLevel = stream.readFloat();
    gateExternalInput = stream.readBool();

    for (int s = 0; s < numModSources; ++s)
        for (int d = 0; d < numModDestinations; ++d)
            modulationMatrix[s][d] = stream.readFloat();
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WestPatchAudioProcessor();
}
