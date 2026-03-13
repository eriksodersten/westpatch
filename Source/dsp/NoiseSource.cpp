#include "NoiseSource.h"

#include <JuceHeader.h>

void NoiseSource::prepare (double) noexcept
{
}

void NoiseSource::reset() noexcept
{
}

float NoiseSource::process() noexcept
{
    return juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
}
