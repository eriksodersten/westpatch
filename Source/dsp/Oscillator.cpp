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

float Oscillator::process (float frequencyHz, float shape) noexcept
{
    const float noise = lcgNoise();

    const float jitter = noise * 0.00008f;

    phase += static_cast<double> (frequencyHz) / sampleRate + jitter;

    if (phase >= 1.0)
        phase -= 1.0;
    if (phase < 0.0)
        phase += 1.0;

    const float p = static_cast<float> (phase);

    // Triangel
    float tri;
    if (phase < 0.5)
        tri = static_cast<float> (4.0 * phase - 1.0);
    else
        tri = static_cast<float> (3.0 - 4.0 * phase);

    // Sinus (från triangel)
    const float sine = triToSine (tri);

    // Sågvåg
    const float saw = 2.0f * p - 1.0f;

    // Interpolera: 0.0 = sinus, 0.5 = triangel, 1.0 = sågvåg
    float output;
    if (shape < 0.5f)
    {
        const float t = shape * 2.0f;
        output = sine * (1.0f - t) + tri * t;
    }
    else
    {
        const float t = (shape - 0.5f) * 2.0f;
        output = tri * (1.0f - t) + saw * t;
    }

    output = std::tanh (output * 1.05f);

    return output + noise * 0.0008f;
}
