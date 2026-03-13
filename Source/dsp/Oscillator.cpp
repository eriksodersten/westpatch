#include "Oscillator.h"

#include <cmath>

void Oscillator::prepare (double newSampleRate) noexcept
{
    sampleRate = newSampleRate;
    reset();
}

void Oscillator::reset() noexcept
{
    phase = 0.0;
}

float Oscillator::process (float frequencyHz) noexcept
{
    phase += frequencyHz / sampleRate;

    if (phase >= 1.0)
        phase -= 1.0;

    return std::sin (2.0 * M_PI * phase);
}
