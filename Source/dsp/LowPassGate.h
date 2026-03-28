#pragma once

class LowPassGate
{
public:
    enum class Mode { Lopass, Gate, Combo };

    void prepare (double sampleRate) noexcept;
    void reset() noexcept;
    void setMode (Mode newMode) noexcept { mode = newMode; }
    Mode getMode() const noexcept { return mode; }

    float process (float input,
                   float cutoffEnvelope,
                   float outputEnvelope,
                   float amount,
                   float cv) noexcept;

private:
    double sampleRate    = 44100.0;

    Mode  mode           = Mode::Combo;
        float vacState       = 0.0f;
        float attackCoeff    = 0.0f;
        float releaseCoeff   = 0.0f;

    float s1             = 0.0f;
    float s2             = 0.0f;
};
