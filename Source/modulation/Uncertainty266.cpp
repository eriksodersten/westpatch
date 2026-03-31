#include "Uncertainty266.h"

#include <algorithm>

//==============================================================================
void Uncertainty266::prepare (double newSampleRate) noexcept
{
    sampleRate = newSampleRate;
    reset();
}

//==============================================================================
void Uncertainty266::reset() noexcept
{
    phase = 0.0f;

    smoothValue = 0.0f;
    smoothTarget = 0.0f;

    steppedValue = 0.0f;
        steppedSlewed = 0.0f;
            prevSteppedTarget = 0.0f;

        biasValue = 0.0f;
        biasTarget = 0.0f;
        prevSmoothTarget = 0.0f;

    pulseValue = 0.0f;

    rngState = 0x12345678u;
}

//==============================================================================
float Uncertainty266::nextUniformBipolar() noexcept
{
    // Cheap linear congruential generator
    rngState = 1664525u * rngState + 1013904223u;

    const float normalized =
        static_cast<float> ((rngState >> 8) & 0x00FFFFFFu) * (1.0f / 16777215.0f);

    return normalized * 2.0f - 1.0f;
}

//==============================================================================
float Uncertainty266::nextCenterWeighted() noexcept
{
    // Triangular distribution
    const float a = nextUniformBipolar();
    const float b = nextUniformBipolar();

    return 0.5f * (a + b);
}

//==============================================================================
Uncertainty266::Outputs Uncertainty266::process (const Params& params) noexcept
{
    Outputs out;

    const float clampedRate        = std::max (0.0f, params.rate);
    const float clampedSmoothAmt   = std::max (0.0f, params.smoothAmount);
    const float clampedSteppedAmt  = std::max (0.0f, params.steppedAmount);
    const float clampedDensity     = std::clamp (params.density, 0.0f, 1.0f);
    const float clampedCorrelation = std::clamp (params.correlation, 0.0f, 1.0f);
    const float clampedSpread      = std::max (0.0f, params.spread);

    //====================================================
    // Clock phase
    phase += clampedRate / static_cast<float> (sampleRate);

    bool event = false;

    if (phase >= 1.0f)
    {
        phase -= 1.0f;

        const float eventChance = 0.5f * (nextUniformBipolar() + 1.0f);
        event = (eventChance <= clampedDensity);
    }

    //====================================================
    // Event update
    if (event)
    {
        // Bias updates less frequently
        const float biasChance = 0.5f * (nextUniformBipolar() + 1.0f);

        if (biasChance < 0.35f)
            biasTarget = nextCenterWeighted() * 0.75f * clampedSpread;

        // Smooth target – korrelation med föregående target (autentisk 266-logik)
                {
                    const float independent = nextCenterWeighted() * clampedSpread;
                    smoothTarget =
                        (independent       * (1.0f - clampedCorrelation)) +
                        (prevSmoothTarget   * clampedCorrelation);
                    prevSmoothTarget = smoothTarget;
                }

                // Stepped value – korrelation med föregående steg
                {
                    const float independent = nextCenterWeighted() * clampedSpread;
                    const float newTarget =
                        (independent        * (1.0f - clampedCorrelation)) +
                        (prevSteppedTarget  * clampedCorrelation);
                    prevSteppedTarget = newTarget;
                    steppedValue = newTarget * clampedSteppedAmt;
                }

        // Trigger pulse
        pulseValue = 1.0f;
    }

    //====================================================
    // Rate-aware drift / slew
    const float normalizedRate = std::clamp (clampedRate / 10.0f, 0.0f, 1.0f);

    const float biasCoeff =
        0.00005f + (0.0010f - 0.00005f) * normalizedRate;

    const float smoothCoeff =
        0.00015f + (0.0030f - 0.00015f) * normalizedRate;

    biasValue += biasCoeff * (biasTarget - biasValue);
    smoothValue += smoothCoeff * (smoothTarget - smoothValue);

    // Pulse decay
    pulseValue *= 0.90f;

    if (pulseValue < 0.0001f)
        pulseValue = 0.0f;

    //====================================================
    // Integrator – slew på stepped-output (autentisk 266-funktion)
        if (params.slewTime > 0.0001f)
        {
            const float coeff = 1.0f - std::exp (-1.0f / (params.slewTime * static_cast<float> (sampleRate)));
            steppedSlewed += coeff * (steppedValue - steppedSlewed);
        }
        else
        {
            steppedSlewed = steppedValue;
        }

        out.smooth  = smoothValue * clampedSmoothAmt;
        out.stepped = steppedSlewed;
    out.bias    = biasValue;
    out.pulse   = pulseValue;

    return out;
}
