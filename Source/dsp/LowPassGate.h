#pragma once

class LowPassGate
{
public:
    void prepare (double sampleRate) noexcept;
    void reset() noexcept;

    float process (float input,
                   float cutoffEnvelope,
                   float outputEnvelope,
                   float amount,
                   float cv) noexcept;

private:
    double sampleRate    = 44100.0;

    float vacState       = 0.0f;
    float attackCoeff    = 0.0f;
    float releaseCoeff   = 0.0f;

    float s1             = 0.0f;
    float s2             = 0.0f;
};
