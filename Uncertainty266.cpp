#include "Uncertainty266.h"

#include <algorithm>

void Uncertainty266::prepare (double newSampleRate) noexcept
{
    sampleRate = newSampleRate;
    reset();
}

void Uncertainty266::reset() noexcept
{
    phase = 0.0f;

    smoothValue = 0.0f;
    smoothTarget = 0.0f;

    steppedValue = 0.0f;

    biasValue = 0.0f;
    biasTarget = 0.0f;

    pulseValue = 0.0f;

    rngState = 0x12345678u;
}

float Uncertainty266::nextUniformBipolar() noexcept
{
    rngState = 1664525u * rngState + 1013904223u;

    const float normalized = static_cast<float> ((rngState >> 8) & 0x00FFFFFFu)
                           * (1.0f / 16777215.0f);

    return normalized * 2.0f - 1.0f;
}

float Uncertainty266::nextCenterWeighted() noexcept
{
    const float a = nextUniformBipolar();
    const float b = nextUniformBipolar();
    return 0.5f * (a + b);
}

Uncertainty266::Outputs Uncertainty266::process (const Params& params) noexcept
{
    Outputs out;

    const float clampedRate        = std::max (0.0f, params.rate);
    const float clampedSmoothAmt   = std::max (0.0f, params.smoothAmount);
    const float clampedSteppedAmt  = std::max (0.0f, params.steppedAmount);
    const float clampedDensity     = std::clamp (params.density, 0.0f, 1.0f);
    const float clampedCorrelation = std::clamp (params.correlation, 0.0f, 1.0f);
    const float clampedSpread      = std::max (0.0f, params.spread);

    phase += clampedRate / static_cast<float> (sampleRate);

    bool event = false;
    if (phase >= 1.0f)
    {
        phase -= 1.0f;

        const float eventChance = 0.5f * (nextUniformBipolar() + 1.0f);
        event = (eventChance <= clampedDensity);
    }

    if (event)
    {
        const float biasChance = 0.5f * (nextUniformBipolar() + 1.0f);
        if (biasChance < 0.35f)
            biasTarget = nextCenterWeighted() * 0.75f * clampedSpread;

        {
            const float independent = nextCenterWeighted() * clampedSpread;
            const float correlated  = biasTarget;
            smoothTarget = (independent * (1.0f - clampedCorrelation))
                         + (correlated  * clampedCorrelation);
        }

        {
            const float independent = nextCenterWeighted() * clampedSpread;
            const float correlated  = biasTarget;
            steppedValue = ((independent * (1.0f - clampedCorrelation))
                          + (correlated  * clampedCorrelation))
                         * clampedSteppedAmt;
        }

        pulseValue = 1.0f;
    }

    biasValue += 0.00075f * (biasTarget - biasValue);
    smoothValue += 0.0025f * (smoothTarget - smoothValue);

    pulseValue *= 0.90f;
    if (pulseValue < 0.0001f)
        pulseValue = 0.0f;

    out.smooth = smoothValue * clampedSmoothAmt;
    out.stepped = steppedValue;
    out.bias = biasValue;
    out.pulse = pulseValue;

    return out;
}
