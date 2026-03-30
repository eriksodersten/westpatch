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

        // DC-blocker state (single-pole highpass ~5 Hz)
        float dcX = 0.0f;
        float dcY = 0.0f;

    void reset()
                {
                    carrierOsc.reset();
                    modOsc.reset();
                    lpg.reset();
                    dcX = 0.0f;
                    dcY = 0.0f;
                }

    void resetDSPState()
                {
                    lpg.reset();
                    dcX = 0.0f;
                    dcY = 0.0f;
                }
    void prepare (double sampleRate);

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
                             float noiseIn,
                             float oscShape = 0.0f)
    {
        const float safeCarrierHz = juce::jmax (20.0f, carrierFrequencyHz);
        const float safeModRatio = juce::jmax (0.01f, modRatio);

        // Mod oscillator follows carrier by ratio
        const float modFrequencyHz = safeCarrierHz * safeModRatio;
        const float modSignal = modOsc.process (modFrequencyHz, oscShape);

        // More musical proportional FM instead of plain additive Hz offset
        const float fmAmount = juce::jmax (0.0f, fmAmountHz);
        const float modulatedCarrierHz =
            safeCarrierHz * juce::jmax (0.01f, 1.0f + (modSignal * (fmAmount / 200.0f)));

        const float carrierSignal = carrierOsc.process (modulatedCarrierHz, oscShape);

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

        // DC-blocker: y[n] = x[n] - x[n-1] + R * y[n-1], R ≈ 1 - (2π*5/44100)
                const float dcIn = synthFolded + noiseFolded;
                constexpr float R = 0.99929f; // 2π*5/44100
                dcY = dcIn - dcX + R * dcY;
                dcX = dcIn;

                return lpg.process (
                    dcY,
            lpgCutoffEnvelope,
            lpgOutputEnvelope,
            lpgAmount,
            lpgCV
        );
    }
};
