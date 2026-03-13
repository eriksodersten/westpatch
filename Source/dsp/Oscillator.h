#pragma once

class Oscillator
{
public:
    void prepare (double newSampleRate) noexcept;
    void reset() noexcept;

    float process (float frequencyHz) noexcept;

private:
    double sampleRate = 44100.0;
    double phase = 0.0;
};
