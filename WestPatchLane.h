#pragma once

#include "Oscillator.h"
#include "Wavefolder.h"
#include "LowPassGate.h"

struct WestPatchLane
{
    Oscillator oscillator;
    Wavefolder wavefolder;
    LowPassGate lpg;

    float frequency = 440.0f;

    void prepare(double sampleRate)
    {
        oscillator.prepare(sampleRate);
        wavefolder.prepare(sampleRate);
        lpg.prepare(sampleRate);
    }

    float render(
        float pitchRatio,
        float foldValue,
        float lpgEnvelope,
        float lpgAmount,
        float lpgCV,
        float synthLevel)
    {
        float osc = oscillator.process(frequency * pitchRatio);

        float folded = wavefolder.process(
            osc * synthLevel,
            foldValue
        );

        float out = lpg.process(
            folded,
            lpgEnvelope,
            lpgAmount,
            lpgCV
        );

        return out;
    }
};
