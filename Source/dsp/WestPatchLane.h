#pragma once

#include <JuceHeader.h>
#include "Oscillator.h"
#include "Wavefolder.h"
#include "LowPassGate.h"

struct WestPatchLane
{
    Oscillator carrierOsc;
    Oscillator modOsc;
    Wavefolder wavefolder;
    LowPassGate lpg;
    float frequency = 440.0f;

    void prepare (double sampleRate)
    {
        carrierOsc.prepare (sampleRate);
        modOsc.prepare (sampleRate);
        lpg.prepare (sampleRate);
    }

    float renderComplex (float carrierFrequencyHz,
                         float modRatio,
                         float fmAmountHz,
                         float oscMix,
                         float synthLevel,
                         float foldValue,
                         float lpgCutoffEnvelope,
                         float lpgOutputEnvelope,
                         float lpgAmount,
                         float lpgCV,
                         float noiseIn)
    {
        const float safeCarrierHz = juce::jmax (20.0f, carrierFrequencyHz);
        const float safeModRatio = juce::jmax (0.01f, modRatio);

        // Mod oscillator follows carrier by ratio
        const float modFrequencyHz = safeCarrierHz * safeModRatio;
        const float modSignal = modOsc.process (modFrequencyHz);

        // More musical proportional FM instead of plain additive Hz offset
        const float fmAmount = juce::jmax (0.0f, fmAmountHz);
        const float modulatedCarrierHz =
            safeCarrierHz * juce::jmax (0.01f, 1.0f + (modSignal * (fmAmount / 200.0f)));

        const float carrierSignal = carrierOsc.process (modulatedCarrierHz);

        // Do not hear the mod osc directly when FM amount is zero
        float clampedMix = juce::jlimit (0.0f, 1.0f, oscMix);
        if (fmAmountHz < 0.0001f)
            clampedMix = 0.0f;

        const float oscSection =
            (carrierSignal * (1.0f - clampedMix)) + (modSignal * clampedMix);

        // Let the mod osc also animate the fold amount a bit
        const float dynamicFold = foldValue + (modSignal * clampedMix * 0.75f);

        const float synthFolded = wavefolder.process (
            oscSection * synthLevel,
            dynamicFold
        );

        const float noiseFolded = wavefolder.process (
            noiseIn,
            dynamicFold
        );

        return lpg.process (
            synthFolded + noiseFolded,
            lpgCutoffEnvelope,
            lpgOutputEnvelope,
            lpgAmount,
            lpgCV
        );
    }
};
