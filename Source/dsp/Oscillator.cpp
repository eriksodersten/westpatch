#include "Oscillator.h"
#include <cmath>
#include <cstdint>

void Oscillator::prepare (double newSampleRate) noexcept
{
    sampleRate = newSampleRate;
    reset();
}

void Oscillator::reset() noexcept
{
    phase = 0.0;
}

float Oscillator::lcgNoise() noexcept
{
    rngState = rngState * 1664525u + 1013904223u;
    return static_cast<float> (static_cast<int32_t> (rngState)) / 2147483648.0f;
}

float Oscillator::triToSine (float tri) noexcept
{
    const float t2 = tri * tri;
    return tri * (1.5708f - t2 * (0.6460f - t2 * 0.0796f));
}

float Oscillator::process (float frequencyHz) noexcept
{
    const float noise = lcgNoise();

    // Fasjitter — ger levande intonation som analogt oscillator-chip
    const float jitter = noise * 0.00008f;

    phase += static_cast<double> (frequencyHz) / sampleRate + jitter;

    if (phase >= 1.0)
        phase -= 1.0;
    if (phase < 0.0)
        phase += 1.0;

    float tri;
    if (phase < 0.5)
        tri = static_cast<float> (4.0 * phase - 1.0);
    else
        tri = static_cast<float> (3.0 - 4.0 * phase);

    const float shaped = std::tanh (triToSine (tri) * 1.05f);

    // Subtilt brus adderat till output — termiskt brus i analog krets
    return shaped + noise * 0.0008f;
}
