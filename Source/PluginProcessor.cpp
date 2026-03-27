#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

namespace
{
    float s_lastRenderedPitchModSemitones = 0.0f;
    float s_lastRenderedFoldModAmount = 0.0f;
    float s_lastRenderedLpgCvMod = 0.0f;
}

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
    foldAmountSmoothed.reset(sampleRate, 0.02);
        foldAmountSmoothed.setCurrentAndTargetValue(foldAmount);
        lpgAmountSmoothed.reset(sampleRate, 0.02);
        lpgAmountSmoothed.setCurrentAndTargetValue(lpgAmount);

    for (auto& lane : lanes)
        lane.prepare (sampleRate);

    noiseSource.prepare (sampleRate);

    functionGenerator281.prepare (sampleRate);
        functionGenerator281.reset();
        functionGenerator281.setMode (func281Mode);

    uncertainty266.prepare (sampleRate);
    uncertainty266.reset();

    groupEnvelopeManager.prepare (sampleRate);
    groupEnvelopeManager.setNumGroups (getNumGroups());
    groupEnvelopeManager.setAttackRelease (attackTime, releaseTime);

    newEngine.prepare (sampleRate);
    newEngine.setAttackRelease (attackTime, releaseTime);

    switch (groupMode)
    {
        case GroupMode::Mono: newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Unison); break;
        case GroupMode::Duo:  newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Duo);    break;
        case GroupMode::Quad: newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Quad);   break;
        default:              newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Unison); break;
    }

    previous266PulseHigh = false;
    resetGroups();
    newEngine.reset();
    newEngine.setAttackRelease (attackTime, releaseTime);
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

void WestPatchAudioProcessor::setGroupMode (GroupMode newMode) noexcept
{
    if (groupMode == newMode)
        return;

    groupMode = newMode;
    groupEnvelopeManager.setNumGroups (getNumGroups());

    switch (groupMode)
    {
        case GroupMode::Mono: newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Unison); break;
        case GroupMode::Duo:  newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Duo);    break;
        case GroupMode::Quad: newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Quad);   break;
        default:              newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Unison); break;
    }

    resetGroups();
}

int WestPatchAudioProcessor::getActiveGroupCount() const noexcept
{
    return newEngine.getActiveGroupCount();
}

int WestPatchAudioProcessor::laneToGroup (int laneIndex) const noexcept
{
    return newEngine.getLaneGroupIndex (laneIndex);
}

int WestPatchAudioProcessor::getNumGroups() const noexcept
{
    return newEngine.getActiveGroupCount();
}

int WestPatchAudioProcessor::findGroupForNoteOn() const noexcept
{
    const int numGroups = getNumGroups();

    // 1) Prefer truly idle groups: not gated and no envelope activity.
    for (int g = 0; g < numGroups; ++g)
    {
        if (! groups[g].gate && ! groupEnvelopeManager.isEnvelopeActive (g))
            return g;
    }

    // 2) Otherwise reuse the oldest releasing group.
    int oldestReleasingGroup = -1;
    std::uint64_t oldestReleasingSerial = 0;

    for (int g = 0; g < numGroups; ++g)
    {
        if (! groups[g].gate && groupEnvelopeManager.isEnvelopeActive (g))
        {
            if (oldestReleasingGroup < 0 || groupAllocationSerial[g] < oldestReleasingSerial)
            {
                oldestReleasingGroup = g;
                oldestReleasingSerial = groupAllocationSerial[g];
            }
        }
    }

    if (oldestReleasingGroup >= 0)
        return oldestReleasingGroup;

    // 3) Finally steal the oldest actively gated group.
    int oldestActiveGroup = 0;
    std::uint64_t oldestActiveSerial = groupAllocationSerial[0];

    for (int g = 1; g < numGroups; ++g)
    {
        if (groupAllocationSerial[g] < oldestActiveSerial)
        {
            oldestActiveSerial = groupAllocationSerial[g];
            oldestActiveGroup = g;
        }
    }

    return oldestActiveGroup;
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
    newEngine.reset();

    switch (groupMode)
    {
        case GroupMode::Mono: newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Unison); break;
        case GroupMode::Duo:  newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Duo);    break;
        case GroupMode::Quad: newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Quad);   break;
        default:              newEngine.setActiveGroupMode (westpatch::engine::ActiveGroupMode::Unison); break;
    }

    newEngine.setAttackRelease (attackTime, releaseTime);

    for (int g = 0; g < maxGroups; ++g)
        crossfades[g] = {};

    for (int g = 0; g < maxGroups; ++g)
        tails[g] = {};

    for (int i = 0; i < numLanes; ++i)
        laneOutputCache[i] = 0.0f;
}

void WestPatchAudioProcessor::noteOnToGroup (int midiNoteNumber) noexcept
{
    const float frequencyHz = static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    const auto newResult = newEngine.beginNoteOn (midiNoteNumber, frequencyHz);
    

    const int groupIndex = juce::jlimit (0, getNumGroups() - 1, newResult.decision.groupIndex);

    DBG ("noteOn decision=" + juce::String((int)newResult.decision.action)
         + " group=" + juce::String(groupIndex)
         + " legacyEnvActive=" + juce::String(groupEnvelopeManager.isEnvelopeActive(groupIndex) ? 1 : 0)
         + " legacyGate=" + juce::String(groups[groupIndex].gate ? 1 : 0));
    auto& group = groups[groupIndex];
    
    const bool stealingActiveGroup =
            newResult.decision.action == westpatch::engine::NoteOnAction::StealActiveGroup;
        const bool reusingReleasingGroup =
            newResult.decision.action == westpatch::engine::NoteOnAction::ReuseReleasingGroup;
    
    const bool sameGroupHandoff = stealingActiveGroup || reusingReleasingGroup;
    
    if (sameGroupHandoff)
    {
        if (reusingReleasingGroup || stealingActiveGroup)
        {
            auto& tail = tails[groupIndex];
            tail = {};
            tail.active = true;
            tail.samplesRemaining = tailReleaseSamples;
            tail.frequencyHz = group.frequencyHz > 0.0f ? group.frequencyHz : 440.0f;
            tail.gain = juce::jlimit (0.0f, 1.0f, lastRenderedGroupEnv[groupIndex]);
            tail.gainStep = tail.gain / static_cast<float> (tailReleaseSamples);
            tail.pitchModSemitones = s_lastRenderedPitchModSemitones;
                    tail.foldModAmount = s_lastRenderedFoldModAmount;
                    tail.lpgCvMod = s_lastRenderedLpgCvMod;

            for (int i = 0; i < numLanes; ++i)
            {
                if (laneToGroup (i) == groupIndex)
                    tail.lanes[i] = lanes[i];
            }

            crossfades[groupIndex] = {};

            DBG ("TAIL START group=" + juce::String (groupIndex)
                 + " steal=" + juce::String (stealingActiveGroup ? 1 : 0)
                 + " gain=" + juce::String (tail.gain)
                 + " freq=" + juce::String (tail.frequencyHz));
        }
        
        handoffDebug.active = true;
        handoffDebug.groupIndex = groupIndex;
        handoffDebug.samplesRemaining = handoffDebugSamples;
        handoffDebug.sampleCounter = 0;
        handoffDebug.oldFrequencyHz = group.frequencyHz;
        handoffDebug.newFrequencyHz =
        static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
        handoffDebug.oldGate = group.gate;
        handoffDebug.oldEnvActive = groupEnvelopeManager.isEnvelopeActive (groupIndex);
        
        DBG ("HANDOFF start"
             + juce::String (" group=") + juce::String (groupIndex)
             + " oldGate=" + juce::String (handoffDebug.oldGate ? 1 : 0)
             + " oldEnvActive=" + juce::String (handoffDebug.oldEnvActive ? 1 : 0)
             + " oldFreq=" + juce::String (handoffDebug.oldFrequencyHz)
             + " newFreq=" + juce::String (handoffDebug.newFrequencyHz));
    }
    
    group.gate = true;
    group.midiNote = midiNoteNumber;
    group.frequencyHz = frequencyHz;
    groupAllocationSerial[groupIndex] = nextAllocationSerial++;

    // groupEnvelopeManager är inte längre source of truth för envelope.
        // newEngine.beginNoteOn() hanterar detta internt.
}

void WestPatchAudioProcessor::noteOffFromGroups (int midiNoteNumber) noexcept
{
    const int groupIndex = newEngine.beginNoteOff (midiNoteNumber);

    if (groupIndex < 0)
        return;

    auto& group = groups[groupIndex];

    group.gate = false;
    group.midiNote = -1;

    // Keep frequencyHz during release so the tail stays on pitch.
    // Keep allocation serial during release so allocator can rank
    // releasing groups deterministically.
    // newEngine.beginNoteOff() hanterar envelope internt.
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
    const int numSamples = buffer.getNumSamples();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    groupEnvelopeManager.setNumGroups (getNumGroups());
    groupEnvelopeManager.setAttackRelease (attackTime, releaseTime);
        newEngine.setAttackRelease (attackTime, releaseTime);
    functionGenerator281.setMode (func281Mode);

    // Preserve incoming MIDI for sample-offset processing,
    // while clearing the outgoing MIDI buffer immediately.
    juce::MidiBuffer incomingMidi;
    incomingMidi.swapWith (midiMessages);

    auto handleMidiMessage = [this] (const juce::MidiMessage& msg)
    {
        if (msg.isNoteOn())
        {
            noteOnToGroup (msg.getNoteNumber());
            functionGenerator281.trigger();
                        functionGenerator281.gate (true);
        }
        else if (msg.isNoteOff())
                {
                    noteOffFromGroups (msg.getNoteNumber());
                    functionGenerator281.gate (false);
                }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            resetGroups();
        }
    };

    juce::MidiBuffer::Iterator midiIterator (incomingMidi);
    juce::MidiMessage currentMidiMessage;
    int currentMidiSample = 0;
    bool hasMidiEvent = midiIterator.getNextEvent (currentMidiMessage, currentMidiSample);

    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    auto* inL = totalNumInputChannels > 0 ? buffer.getReadPointer (0) : nullptr;
    auto* inR = totalNumInputChannels > 1 ? buffer.getReadPointer (1) : nullptr;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        while (hasMidiEvent && currentMidiSample <= sample)
        {
            handleMidiMessage (currentMidiMessage);
            hasMidiEvent = midiIterator.getNextEvent (currentMidiMessage, currentMidiSample);
        }

        float inputSample = 0.0f;

        if (inL != nullptr && inR != nullptr)
            inputSample = 0.5f * (inL[sample] + inR[sample]);
        else if (inL != nullptr)
            inputSample = inL[sample];

        float outL = 0.0f;
        float outR = 0.0f;

        renderSample (inputSample, outL, outR);

        preHandoffRingBuffer[preHandoffRingIndex % 10] = outL;
                ++preHandoffRingIndex;
                
                if (fullBlockDebugCount > 0)
                {
                    DBG ("BLOCK sample=" + juce::String (sample) + " sumL=" + juce::String (outL));
                    --fullBlockDebugCount;
                }
                
                left[sample] = outL;

        if (right != nullptr)
            right[sample] = outR;
    }

    // Handle any trailing events at or beyond block end defensively.
    while (hasMidiEvent)
    {
        handleMidiMessage (currentMidiMessage);
        hasMidiEvent = midiIterator.getNextEvent (currentMidiMessage, currentMidiSample);
    }
}

void WestPatchAudioProcessor::renderSample (float inputSample, float& outL, float& outR) noexcept
{
    foldAmountSmoothed.setTargetValue(foldAmount);
        lpgAmountSmoothed.setTargetValue(lpgAmount);
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

    s_lastRenderedPitchModSemitones = pitchModSemitones;
     s_lastRenderedFoldModAmount = foldModAmount;
     s_lastRenderedLpgCvMod = lpgCvMod;

    
    float sumL = 0.0f;
    float sumR = 0.0f;
    
    float groupEnvValues[maxGroups] = { 0.0f, 0.0f, 0.0f, 0.0f };

    for (int g = 0; g < getNumGroups(); ++g)

    {

        groupEnvValues[g] =
                    juce::jlimit(0.0f, 1.0f, newEngine.getNextEnvelopeSample(g));

                lastRenderedGroupEnv[g] = groupEnvValues[g];

    }
    
    float debugGroupPreAmp = 0.0f;
    float debugGroupPostAmp = 0.0f;

    for (int g = 0; g < getNumGroups(); ++g)
    {
        auto& tail = tails[g];
        if (! tail.active)
            continue;

        const float tailBaseFreq = tail.frequencyHz > 0.0f ? tail.frequencyHz : 440.0f;

        for (int laneIndex = 0; laneIndex < numLanes; ++laneIndex)
        {
            if (laneToGroup (laneIndex) != g)
                continue;

            const float detuneSemitones = laneDetuneBase[laneIndex] * detuneAmount;
            const float lanePitchSemitones = detuneSemitones + tail.pitchModSemitones;
            const float laneFreqHz = tailBaseFreq * std::pow (2.0f, lanePitchSemitones / 12.0f);

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

            constexpr float groupToneCurve = 0.90f;

            const float shapedTailToneEnv =
                std::pow (juce::jlimit (0.0f, 1.0f, tail.gain), groupToneCurve);

            const float laneLpgCutoffEnv =
                juce::jlimit (0.0f, 1.0f, shapedTailToneEnv * toneModeBase);

            const float laneLpgOutputEnv = toneModeBase;

            float laneOut = tail.lanes[laneIndex].renderComplex (
                laneFreqHz,
                complexModRatio,
                complexFmAmount,
                complexOscMix,
                synthLevel,
                juce::jmax (0.01f, foldAmountSmoothed.getCurrentValue() + tail.foldModAmount),
                laneLpgCutoffEnv,
                laneLpgOutputEnv,
                lpgAmountSmoothed.getNextValue(),
                tail.lpgCvMod,
                0.0f);

            laneOut *= tail.gain;

            const float pan = juce::jlimit (-1.0f, 1.0f, lanePanBase[laneIndex] * stereoSpread);
            const float leftGain  = std::sqrt (0.5f * (1.0f - pan));
            const float rightGain = std::sqrt (0.5f * (1.0f + pan));

            sumL += laneOut * leftGain;
            sumR += laneOut * rightGain;
        }
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

        const float foldValue = juce::jmax (0.01f, foldAmountSmoothed.getNextValue() + foldModAmount);
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

        constexpr float groupToneCurve = 0.90f;

        // groupEnv driver LPG-cutoff – vacState lägger till vactrol-tröghet ovanpå
                const float laneLpgCutoffEnv = juce::jlimit (0.0f, 1.0f, groupEnv * toneModeBase);
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
            lpgAmountSmoothed.getNextValue(),
            lpgCvMod,
            noiseIn);

            const float laneOutPreAmp = laneOut;

            // First group-aware route: ADSR -> amp

        laneOut *= groupEnv;

                // Spara post-envelope signal som crossfade-referens
                laneOutputCache[laneIndex] = laneOut;
        
        // Crossfade vid handoff för att undvika fasklick
                auto& cf = crossfades[groupIndex];
                if (cf.active)
                {
                    const float t = 1.0f - (static_cast<float> (cf.samplesRemaining)
                                            / static_cast<float> (crossfadeSamples));
                    const float fadeIn  = std::sin (t * juce::MathConstants<float>::halfPi);
                    const float fadeOut = std::cos (t * juce::MathConstants<float>::halfPi);
                    laneOut = laneOut * fadeIn + cf.oldSignal[laneIndex] * fadeOut;
                }
                const float laneOutPostAmp = laneOut;

        if (handoffDebug.active && groupIndex == handoffDebug.groupIndex)
        {
            debugGroupPreAmp += laneOutPreAmp;
            debugGroupPostAmp += laneOutPostAmp;
        }


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

    for (int g = 0; g < getNumGroups(); ++g)
    {
        auto& tail = tails[g];
        if (tail.active)
        {
            tail.gain = juce::jmax (0.0f, tail.gain - tail.gainStep);
            --tail.samplesRemaining;

            if (tail.samplesRemaining <= 0 || tail.gain <= 0.0f)
                tail.active = false;
        }
    }

    for (int g = 0; g < getNumGroups(); ++g)
    {
        if (crossfades[g].active)
        {
            --crossfades[g].samplesRemaining;
            if (crossfades[g].samplesRemaining <= 0)
                crossfades[g].active = false;
        }
    }
    
    if (handoffDebug.active)
    {
        const int g = handoffDebug.groupIndex;
        DBG ("HANDOFF sample="
             + juce::String (handoffDebug.sampleCounter)
             + " group=" + juce::String (g)
             + " env=" + juce::String (groupEnvValues[g])
             + " freq=" + juce::String (groups[g].frequencyHz)
             + " preAmp=" + juce::String (debugGroupPreAmp)
             + " postAmp=" + juce::String (debugGroupPostAmp));

        ++handoffDebug.sampleCounter;
        --handoffDebug.samplesRemaining;

        if (handoffDebug.samplesRemaining <= 0)
        {
            DBG ("HANDOFF end");
            handoffDebug = {};
        }
    }
    
    if (preHandoffDebugCount > 0)
        {
            DBG ("PRE-HANDOFF sumL=" + juce::String (sumL));
            --preHandoffDebugCount;
        }
        
        if (handoffDebug.active)
                DBG ("RENDERSAMPLE sumL=" + juce::String (sumL));        outL = sumL * outputLevel;
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
    stream.writeBool (false);

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
    stream.readBool(); // legacy placeholder

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
        newEngine.setAttackRelease (attackTime, releaseTime);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WestPatchAudioProcessor();
}
