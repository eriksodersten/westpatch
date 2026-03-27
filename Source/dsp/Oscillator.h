#pragma once
#include <cstdint>

class Oscillator
{
public:
    void prepare (double newSampleRate) noexcept;
    void reset() noexcept;
    float process (float frequencyHz) noexcept;

private:
    static float triToSine (float tri) noexcept;
    float lcgNoise() noexcept;

    double sampleRate = 44100.0;
    double phase = 0.0;
    uint32_t rngState = 12345;
};
