#pragma once

class LowPassGate
{
public:
    void prepare (double sampleRate) noexcept;
    void reset() noexcept;

    float process (float input,
                   float envelope,
                   float amount,
                   float cv) noexcept;

private:
    double sampleRate = 44100.0;
    float state = 0.0f;
};
