#include "LowPassGate.h"

#include <algorithm>

void LowPassGate::prepare (double newSampleRate) noexcept
{
    sampleRate = newSampleRate;
    reset();
}

void LowPassGate::reset() noexcept
{
    state = 0.0f;
}

float LowPassGate::process (float input,
                            float cutoffEnvelope,
                            float outputEnvelope,
                            float amount,
                            float cv) noexcept
{
    float cutoff = 0.001f + (cutoffEnvelope * amount) + cv;
    cutoff = std::clamp (cutoff, 0.001f, 1.0f);

    state += cutoff * (input - state);
    return state * outputEnvelope;
}
