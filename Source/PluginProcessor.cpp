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

    resetGroups();
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

//==============================================================================
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
void WestPatchAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;

    for (auto& lane : lanes)
        lane.prepare (sampleRate);

    noiseSource.prepare (sampleRate);

    functionGenerator281.prepare (sampleRate);
    functionGenerator281.reset();
    functionGenerator281.setCycle (funcBCycle);

    uncertainty266.prepare (sampleRate);
    uncertainty266.reset();

    groupEnvelopeManager.prepare (sampleRate);
    groupEnvelopeManager.setNumGroups (getNumGroups());
    groupEnvelopeManager.setAttackRelease (attackTime, releaseTime);

    previous266PulseHigh = false;
    resetGroups();
}

void WestPatchAudioProcessor::releaseResources()
{
}

#if ! JucePlugin_IsMidiEffect
bool WestPatchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono()
        && mainOut != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainInputChannelSet() != mainOut)
        return false;
   #endif

    return true;
}
#endif

//==============================================================================
void WestPatchAudioProcessor::setGroupMode (GroupMode newMode) noexcept
{
    if (groupMode == newMode)
        return;

    groupMode = newMode;
    groupEnvelopeManager.setNumGroups (getNumGroups());
    resetGroups();
}

int WestPatchAudioProcessor::getActiveGroupCount() const noexcept
{
    return getNumGroups();
}

int WestPatchAudioProcessor::laneToGroup (int laneIndex) const noexcept
{
    switch (groupMode)
    {
        case GroupMode::Mono: return 0;
        case GroupMode::Duo:  return laneIndex < 2 ? 0 : 1;
        case GroupMode::Quad: return juce::jlimit (0, maxGroups - 1, laneIndex);
        default:              return 0;
    }
}

int WestPatchAudioProcessor::getNumGroups() const noexcept
{
    switch (groupMode)
    {
        case GroupMode::Mono: return 1;
        case GroupMode::Duo:  return 2;
        case GroupMode::Quad: return 4;
        default:              return 1;
    }
}

int WestPatchAudioProcessor::findGroupForNoteOn() const noexcept
{
    const int numGroups = getNumGroups();

    for (int g = 0; g < numGroups; ++g)
    {
        if (! groups[g].gate)
            return g;
    }

    int oldestGroup = 0;
    std::uint64_t oldestSerial = groupAllocationSerial[0];

    for (int g = 1; g < numGroups; ++g)
    {
        if (groupAllocationSerial[g] < oldestSerial)
        {
            oldestSerial = groupAllocationSerial[g];
            oldestGroup = g;
        }
    }

    return oldestGroup;
}

int WestPatchAudioProcessor::findGroupForNoteOff (int midiNoteNumber) const noexcept
{
    int bestGroup = -1;
    std::uint64_t newestSerial = 0;

    for (int g = 0; g < getNumGroups(); ++g)
    {
        const auto& group = groups[g];

        if (group.gate && group.midiNote == midiNoteNumber)
        {
            if (bestGroup < 0 || groupAllocationSerial[g] > newestSerial)
            {
                bestGroup = g;
                newestSerial = groupAllocationSerial[g];
            }
        }
    }

    return bestGroup;
}

void WestPatchAudioProcessor::resetGroups() noexcept
{
    for (int g = 0; g < maxGroups; ++g)
    {
        groups[g].gate = false;
        groups[g].midiNote = -1;
        groups[g].frequencyHz = 440.0f;
        groupAllocationSerial[g] = 0;
    }

    nextAllocationSerial = 1;
    groupEnvelopeManager.reset();
}

void WestPatchAudioProcessor::noteOnToGroup (int midiNoteNumber) noexcept
{
    const int groupIndex = juce::jlimit (0, getNumGroups() - 1, findGroupForNoteOn());
    auto& group = groups[groupIndex];

    const bool stealingActiveGroup = group.gate;

    if (stealingActiveGroup)
        groupEnvelopeManager.noteOff (groupIndex);

    group.gate = true;
    group.midiNote = midiNoteNumber;
    group.frequencyHz = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    groupAllocationSerial[groupIndex] = nextAllocationSerial++;

    groupEnvelopeManager.noteOn (groupIndex);
}

void WestPatchAudioProcessor::noteOffFromGroups (int midiNoteNumber) noexcept
{
    const int groupIndex = findGroupForNoteOff (midiNoteNumber);

    if (groupIndex < 0)
        return;

    auto& group = groups[groupIndex];

    group.gate = false;
    group.midiNote = -1;
    groupAllocationSerial[groupIndex] = 0;

    groupEnvelopeManager.noteOff (groupIndex);
}

//==============================================================================
bool WestPatchAudioProcessor::hasActiveRoutingForDestination (ModDestination destination) const
{
    const int d = static_cast<int> (destination);

    for (int s = 0; s < numModSources; ++s)
    {
        if (std::abs (modulationMatrix[s][d]) > 0.00001f)
            return true;
    }

    return false;
}

float WestPatchAudioProcessor::mapSteppedRandomToQuantizedSemitoneOffset (float steppedValue,
                                                                          float depthNormalized) noexcept
{
    const float clampedDepth = juce::jlimit (0.0f, 1.0f, depthNormalized);
    const int maxSemitone = static_cast<int> (std::round (clampedDepth * 24.0f));

    if (maxSemitone <= 0)
        return 0.0f;

    const float normalized = juce::jlimit (-1.0f, 1.0f, steppedValue);
    const int quantized = static_cast<int> (std::round (normalized * static_cast<float> (maxSemitone)));

    return static_cast<float> (quantized);
}

//==============================================================================
void WestPatchAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    groupEnvelopeManager.setNumGroups (getNumGroups());
    groupEnvelopeManager.setAttackRelease (attackTime, releaseTime);
    functionGenerator281.setCycle (funcBCycle);

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            noteOnToGroup (msg.getNoteNumber());
            functionGenerator281.trigger(); // remains global
        }
        else if (msg.isNoteOff())
        {
            noteOffFromGroups (msg.getNoteNumber());
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            resetGroups();
        }
    }

    midiMessages.clear();

    auto* left  = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    auto* inL = totalNumInputChannels > 0 ? buffer.getReadPointer (0) : nullptr;
    auto* inR = totalNumInputChannels > 1 ? buffer.getReadPointer (1) : nullptr;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float inputSample = 0.0f;

        if (inL != nullptr && inR != nullptr)
            inputSample = 0.5f * (inL[sample] + inR[sample]);
        else if (inL != nullptr)
            inputSample = inL[sample];

        float outL = 0.0f;
        float outR = 0.0f;

        renderSample (inputSample, outL, outR);

        left[sample] = outL;
        if (right != nullptr)
            right[sample] = outR;
    }
}

void WestPatchAudioProcessor::renderSample (float inputSample, float& outL, float& outR) noexcept
{
    SignalBus bus;

    // 281 stays global
    const float global281Env = functionGenerator281.process (modAttackTime, modReleaseTime);
    bus.modEnv = global281Env;

    // 266 stays global
    Uncertainty266::Params uncertaintyParams;
    uncertaintyParams.rate = uncertaintyRate;
    uncertaintyParams.smoothAmount = uncertaintySmoothDepth;
    uncertaintyParams.steppedAmount = uncertaintySteppedDepth;
    uncertaintyParams.density = 0.5f;
    uncertaintyParams.correlation = 0.5f;
    uncertaintyParams.spread = 1.0f;

    const auto uncertaintyOut = uncertainty266.process (uncertaintyParams);

    const bool pulseHigh = uncertaintyOut.pulse > 0.5f;
    if (pulseHigh && ! previous266PulseHigh && test266PulseToTrigger > 0.001f)
        functionGenerator281.trigger();
    previous266PulseHigh = pulseHigh;

    const float modSources[numModSources] =
    {
        bus.modEnv,
        uncertaintyOut.smooth,
        uncertaintyOut.stepped
    };

    float pitchModSemitones = 0.0f;
    float foldModAmount = 0.0f;
    float lpgCvMod = 0.0f;

    for (int s = 0; s < numModSources; ++s)
    {
        pitchModSemitones += modSources[s] * modulationMatrix[s][static_cast<int> (ModDestination::Pitch)];
        foldModAmount     += modSources[s] * modulationMatrix[s][static_cast<int> (ModDestination::Fold)];
        lpgCvMod          += modSources[s] * modulationMatrix[s][static_cast<int> (ModDestination::LPG)];
    }

    foldModAmount += uncertaintyOut.smooth * test266SmoothToFold;
    pitchModSemitones += mapSteppedRandomToQuantizedSemitoneOffset (uncertaintyOut.stepped,
                                                                    test266SteppedToPitch);
    lpgCvMod += uncertaintyOut.bias * test266BiasToLpg;

    float sumL = 0.0f;
    float sumR = 0.0f;
    
    float groupEnvValues[maxGroups] = { 0.0f, 0.0f, 0.0f, 0.0f };

    for (int g = 0; g < getNumGroups(); ++g)
    {
        groupEnvValues[g] =
            juce::jlimit(0.0f, 1.0f, groupEnvelopeManager.getNextSample(g));
    }

    for (int laneIndex = 0; laneIndex < numLanes; ++laneIndex)
    {
        const int groupIndex = laneToGroup (laneIndex);
        const auto& group = groups[groupIndex];

        const float groupEnv = groupEnvValues[groupIndex];

        const float baseFreq = group.frequencyHz > 0.0f ? group.frequencyHz : 440.0f;

        const float detuneSemitones = laneDetuneBase[laneIndex] * detuneAmount;
        const float lanePitchSemitones = detuneSemitones + pitchModSemitones;
        const float laneFreqHz = baseFreq * std::pow (2.0f, lanePitchSemitones / 12.0f);

        const float foldValue = juce::jmax (0.0f, foldAmount + foldModAmount);
        const float noiseIn = noiseSource.process() * noiseLevel;

        float toneModeBase = 1.0f;

        switch (toneMode)
        {
            case ToneMode::West:
                toneModeBase = 1.0f;
                break;
            case ToneMode::Moog:
                toneModeBase = 0.9f;
                break;
            case ToneMode::Roland:
                toneModeBase = 0.8f;
                break;
            default:
                toneModeBase = 1.0f;
                break;
        }

        const float laneLpgCutoffEnv =
            juce::jlimit (0.0f, 1.0f, groupEnv * toneModeBase);

        // Behåll tone-mode output scaling ungefär som idag,
        // men låt group envelope styra cutoff/tone.
        const float laneLpgOutputEnv = toneModeBase;

        float laneOut = lanes[laneIndex].renderComplex (
            laneFreqHz,
            complexModRatio,
            complexFmAmount,
            complexOscMix,
            synthLevel,
            foldValue,
            laneLpgCutoffEnv,
            laneLpgOutputEnv,
            lpgAmount,
            lpgCvMod,
            noiseIn
        );

        // First group-aware route: ADSR -> amp
        laneOut *= groupEnv;


        float ext = inputSample * inputLevel;
        if (gateExternalInput)
            ext *= groupEnv;

        laneOut += ext * 0.25f;

        const float pan = juce::jlimit (-1.0f, 1.0f, lanePanBase[laneIndex] * stereoSpread);
        const float leftGain  = std::sqrt (0.5f * (1.0f - pan));
        const float rightGain = std::sqrt (0.5f * (1.0f + pan));

        sumL += laneOut * leftGain;
        sumR += laneOut * rightGain;
    }

    outL = sumL * outputLevel;
    outR = sumR * outputLevel;
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
    juce::MemoryOutputStream stream (destData, true);

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

    stream.writeFloat (test266SmoothToFold);
    stream.writeFloat (test266SteppedToPitch);
    stream.writeFloat (test266BiasToLpg);
    stream.writeFloat (test266PulseToTrigger);

    stream.writeFloat (detuneAmount);
    stream.writeFloat (stereoSpread);
    stream.writeFloat (complexModRatio);
    stream.writeFloat (complexFmAmount);
    stream.writeFloat (complexOscMix);

    stream.writeInt (static_cast<int> (groupMode));
    stream.writeInt (static_cast<int> (toneMode));

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

    test266SmoothToFold = stream.readFloat();
    test266SteppedToPitch = stream.readFloat();
    test266BiasToLpg = stream.readFloat();
    test266PulseToTrigger = stream.readFloat();

    detuneAmount = stream.readFloat();
    stereoSpread = stream.readFloat();
    complexModRatio = stream.readFloat();
    complexFmAmount = stream.readFloat();
    complexOscMix = stream.readFloat();

    const int storedGroupMode = stream.readInt();
    switch (storedGroupMode)
    {
        case 0: groupMode = GroupMode::Mono; break;
        case 1: groupMode = GroupMode::Duo;  break;
        case 2: groupMode = GroupMode::Quad; break;
        default: groupMode = GroupMode::Mono; break;
    }

    const int storedToneMode = stream.readInt();
    switch (storedToneMode)
    {
        case 0: toneMode = ToneMode::West;   break;
        case 1: toneMode = ToneMode::Moog;   break;
        case 2: toneMode = ToneMode::Roland; break;
        default: toneMode = ToneMode::West;  break;
    }

    for (int s = 0; s < numModSources; ++s)
    {
        for (int d = 0; d < numModDestinations; ++d)
        {
            if (! stream.isExhausted())
                modulationMatrix[s][d] = stream.readFloat();
        }
    }

    groupEnvelopeManager.setNumGroups (getNumGroups());
    groupEnvelopeManager.setAttackRelease (attackTime, releaseTime);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WestPatchAudioProcessor();
}
