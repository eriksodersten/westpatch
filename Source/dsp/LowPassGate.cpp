#include "LowPassGate.h"
#include <cmath>
#include <algorithm>

void LowPassGate::prepare (double newSampleRate) noexcept
{
    sampleRate = newSampleRate;

    attackCoeff  = 1.0f - std::exp (-1.0f / (0.012f * (float) sampleRate));
    releaseCoeff = 1.0f - std::exp (-1.0f / (0.250f * (float) sampleRate));

    reset();
}

void LowPassGate::reset() noexcept
{
    vacState = 0.0f;
    s1       = 0.0f;
    s2       = 0.0f;
}

float LowPassGate::process (float input,
                            float cutoffEnvelope,
                            float outputEnvelope,
                            float amount,
                            float cv) noexcept
{
    // --- Mode-beroende routing ---
        float filterEnv = cutoffEnvelope;
        float ampEnv    = outputEnvelope;

        switch (mode)
        {
            case Mode::Lopass:
                filterEnv = cutoffEnvelope;
                ampEnv    = 1.0f;
                break;
            case Mode::Gate:
                filterEnv = 1.0f;
                ampEnv    = outputEnvelope;
                break;
            case Mode::Combo:
                filterEnv = cutoffEnvelope;
                ampEnv    = outputEnvelope;
                break;
        }

        // --- Vactrol-state ---
        const float target = std::clamp (filterEnv + cv, 0.0f, 1.0f);
        const float coeff  = target > vacState ? attackCoeff : releaseCoeff;
        vacState = vacState + coeff * (target - vacState);

        // --- Cutoff: exponentiell ur vacState ---
        const float maxCutoff = 0.49f * (float) sampleRate;
        const float cutoffHz  = std::clamp (30.0f * std::exp (vacState * amount * std::log (333.0f)),
                                            20.0f, maxCutoff);

        // --- TPT SVF ---
        const float q  = 0.5f + amount * 0.6f;
        const float k  = 1.0f / q;
        const float g  = std::tan (3.14159265f * cutoffHz / (float) sampleRate);
        const float a1 = 1.0f / (1.0f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;

        const float v3 = input - s2;
        const float v1 = a1 * s1 + a2 * v3;
        const float v2 = s2 + a2 * s1 + a3 * v3;

        s1 = std::clamp (2.0f * v1 - s1, -4.0f, 4.0f);
        s2 = std::clamp (2.0f * v2 - s2, -4.0f, 4.0f);

        return v2 * ampEnv;
}
