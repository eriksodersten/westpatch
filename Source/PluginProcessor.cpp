#include "PluginProcessor.h"
#include "PluginEditor.h"

WestPatchAudioProcessor::WestPatchAudioProcessor()
     : AudioProcessor (
         BusesProperties()
             .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
             .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    modulationMatrix[(int) ModSource::FunctionB][(int) ModDestination::Pitch] = 1.0f;
    modulationMatrix[(int) ModSource::UncertaintySmooth][(int) ModDestination::Fold] = 1.0f;
    modulationMatrix[(int) ModSource::UncertaintyStepped][(int) ModDestination::Pitch] = 1.0f;
}

WestPatchAudioProcessor::~WestPatchAudioProcessor() {}

const juce::String WestPatchAudioProcessor::getName() const { return JucePlugin_Name; }

bool WestPatchAudioProcessor::acceptsMidi() const { return true; }
bool WestPatchAudioProcessor::producesMidi() const { return false; }
bool WestPatchAudioProcessor::isMidiEffect() const { return false; }

double WestPatchAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int WestPatchAudioProcessor::getNumPrograms() { return 1; }
int WestPatchAudioProcessor::getCurrentProgram() { return 0; }
void WestPatchAudioProcessor::setCurrentProgram (int) {}
const juce::String WestPatchAudioProcessor::getProgramName (int) { return {}; }
void WestPatchAudioProcessor::changeProgramName (int, const juce::String&) {}

void WestPatchAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;

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

    smoothRandomValue = 0.0f;
    smoothRandomTarget = 0.0f;
    steppedRandomValue = 0.0f;
    uncertaintyPhase = 0.0f;
    smoothedSynthLevel = synthLevel;
    smoothedInputLevel = inputLevel;
    smoothedNoiseLevel = noiseLevel;
}

void WestPatchAudioProcessor::releaseResources() {}

bool WestPatchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void WestPatchAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            currentMidiNote = msg.getNoteNumber();
            frequency = juce::MidiMessage::getMidiNoteInHertz(currentMidiNote);
            isNoteOn = true;
        }
        else if (msg.isNoteOff())
        {
            if (msg.getNoteNumber() == currentMidiNote)
                isNoteOn = false;
        }
    }

    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();
    auto totalNumInputChannels = getTotalNumInputChannels();
    
    const float smoothing = 0.0015f;

    const float attackStep = 1.0f / (attackTime * static_cast<float>(currentSampleRate));
    const float releaseStep = 1.0f / (releaseTime * static_cast<float>(currentSampleRate));
    const float modAttackStep = 1.0f / (modAttackTime * static_cast<float>(currentSampleRate));
    const float modReleaseStep = 1.0f / (modReleaseTime * static_cast<float>(currentSampleRate));

 

    for (int sample = 0; sample < numSamples; ++sample)
    {
        smoothedFoldAmount += (foldAmount - smoothedFoldAmount) * smoothing;
        smoothedAttackTime += (attackTime - smoothedAttackTime) * smoothing;
        smoothedSynthLevel += (synthLevel - smoothedSynthLevel) * smoothing;
        smoothedInputLevel += (inputLevel - smoothedInputLevel) * smoothing;
        smoothedReleaseTime += (releaseTime - smoothedReleaseTime) * smoothing;
        smoothedLpgAmount += (lpgAmount - smoothedLpgAmount) * smoothing;
        smoothedModDepth += (modDepth - smoothedModDepth) * smoothing;
        smoothedFuncBRate += (funcBRate - smoothedFuncBRate) * smoothing;
        smoothedFuncBDepth += (funcBDepth - smoothedFuncBDepth) * smoothing;
        smoothedUncertaintyRate += (uncertaintyRate - smoothedUncertaintyRate) * smoothing;
        smoothedUncertaintySmoothDepth += (uncertaintySmoothDepth - smoothedUncertaintySmoothDepth) * smoothing;
        smoothedUncertaintySteppedDepth += (uncertaintySteppedDepth - smoothedUncertaintySteppedDepth) * smoothing;
        smoothedNoiseLevel += (noiseLevel - smoothedNoiseLevel) * smoothing;

        float funcBValue = 0.0f;

        if (funcBCycle)
        {
            funcBPhase += smoothedFuncBRate / static_cast<float>(currentSampleRate);

            if (funcBPhase >= 1.0f)
                funcBPhase -= 1.0f;

            if (funcBPhase < 0.5f)
                funcBValue = funcBPhase * 2.0f;
            else
                funcBValue = 2.0f - (funcBPhase * 2.0f);
        }

        // ===== 266 Source of Uncertainty =====

        uncertaintyPhase += smoothedUncertaintyRate / static_cast<float>(currentSampleRate);

        if (uncertaintyPhase >= 1.0f)
        {
            uncertaintyPhase -= 1.0f;

            smoothRandomTarget = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
            steppedRandomValue = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
        }

        smoothRandomValue += (smoothRandomTarget - smoothRandomValue) * 0.0025f;

        if (isNoteOn)
        {
            envelope += attackStep;
            modEnvelope += modAttackStep;
        }
        else
        {
            envelope -= releaseStep;
            modEnvelope -= modReleaseStep;
        }

        envelope = juce::jlimit(0.0f, 1.0f, envelope);
        modEnvelope = juce::jlimit(0.0f, 1.0f, modEnvelope);

        float pitchCV = 0.0f;
        float foldCV = 0.0f;
        float lpgCV = 0.0f;

        const bool pitchPatched = hasActiveRoutingForDestination (ModDestination::Pitch);
        const bool foldPatched  = hasActiveRoutingForDestination (ModDestination::Fold);
        const bool lpgPatched   = hasActiveRoutingForDestination (ModDestination::LPG);
        
        const float sourceFunctionB = (funcBValue - 0.5f) * 2.0f * smoothedFuncBDepth;
        const float sourceUncertaintySmooth = smoothRandomValue * smoothedUncertaintySmoothDepth;
        const float sourceUncertaintyStepped = steppedRandomValue * smoothedUncertaintySteppedDepth;
        if (! pitchPatched)
        {
            pitchCV += sourceFunctionB;
            pitchCV += sourceUncertaintyStepped;
        }

        if (! foldPatched)
        {
            foldCV += modEnvelope * smoothedModDepth;
            foldCV += sourceUncertaintySmooth;
        }

        if (! lpgPatched)
        {
            lpgCV += 0.0f;
        }
        
        const float sourceValues[numModSources] =
        {
            sourceFunctionB,
            sourceUncertaintySmooth,
            sourceUncertaintyStepped
        };

        for (int source = 0; source < numModSources; ++source)
        {
            if (pitchPatched)
                pitchCV += sourceValues[source] * modulationMatrix[source][(int) ModDestination::Pitch];

            if (foldPatched)
                foldCV += sourceValues[source] * modulationMatrix[source][(int) ModDestination::Fold];

            if (lpgPatched)
                lpgCV += sourceValues[source] * modulationMatrix[source][(int) ModDestination::LPG];
        }

        const float pitchOffsetHz = ((funcBValue - 0.5f) * 2.0f * smoothedFuncBDepth)
                                  + (steppedRandomValue * smoothedUncertaintySteppedDepth);
        const float modulatedFrequency = juce::jmax(20.0f, frequency + pitchOffsetHz);
        const double phaseIncrement = (2.0 * juce::MathConstants<double>::pi * modulatedFrequency) / currentSampleRate;
        
        float inputSample = 0.0f;

        if (totalNumInputChannels > 0)
        {
            const float inL = buffer.getSample (0, sample);
            const float inR = totalNumInputChannels > 1 ? buffer.getSample (1, sample) : inL;
            inputSample = 0.5f * (inL + inR);
        }

        const float synthSample = std::sin (phase) * smoothedSynthLevel;
        const float extSample = inputSample * smoothedInputLevel;
        const float noiseSample = (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * smoothedNoiseLevel;

        const float synthRaw = synthSample;
        const float extRaw = extSample;
        const float noiseRaw = noiseSample;

        const float synthFolded = wavefolder.process(
            synthRaw,
            smoothedFoldAmount + foldCV
        );

        const float extFolded = wavefolder.process(
            extRaw,
            smoothedFoldAmount + foldCV
        );

        const float noiseFolded = wavefolder.process(
            noiseRaw,
            smoothedFoldAmount + foldCV
        );

        lpgCutoff = 0.001f + (envelope * smoothedLpgAmount) + lpgCV;
        lpgCutoff = juce::jlimit(0.001f, 1.0f, lpgCutoff);

        const float synthBus = synthFolded + noiseFolded;
        lpgState += lpgCutoff * (synthBus - lpgState);

        float extOut = 0.0f;

        if (gateExternalInput)
            extOut = extFolded * envelope;
        else
            extOut = extFolded;

        const float value = (lpgState * envelope + extOut) * outputLevel;

        phase += phaseIncrement;
        if (phase > 2.0 * juce::MathConstants<double>::pi)
            phase -= 2.0 * juce::MathConstants<double>::pi;

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.setSample(channel, sample, value);
    }
}

bool WestPatchAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* WestPatchAudioProcessor::createEditor()
{
    return new WestPatchAudioProcessorEditor (*this);
}

void WestPatchAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void WestPatchAudioProcessor::setStateInformation (const void*, int) {}

bool WestPatchAudioProcessor::hasActiveRoutingForDestination (ModDestination destination) const
{
    const int dest = (int) destination;

    for (int source = 0; source < numModSources; ++source)
    {
        if (std::abs (modulationMatrix[source][dest]) > 0.0001f)
            return true;
    }

    return false;
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WestPatchAudioProcessor();
}
