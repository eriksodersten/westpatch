#pragma once

class NoiseSource
{
public:
    void prepare (double sampleRate) noexcept;
    void reset() noexcept;

    float process() noexcept;
};
