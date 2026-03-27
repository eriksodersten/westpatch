#pragma once

class FunctionGenerator281
{
public:
    enum class Mode
    {
        Transient,  // AR  – decay startar direkt efter attack
        Sustain,    // ASR – håller på toppen tills gate går låg
        Cycle       // LFO – decay slutar → attack startar direkt
    };

    void prepare (double newSampleRate) noexcept;
    void reset() noexcept;

    void setMode (Mode newMode) noexcept;
    void trigger() noexcept;
    void gate (bool high) noexcept;
    bool getEndPulse() const noexcept;

    float process (float attackTimeSeconds,
                   float decayTimeSeconds) noexcept;

private:
    enum class Stage
    {
        Idle,
        Attack,
        Sustain,
        Decay
    };

    void enterAttack() noexcept;
    void enterDecay() noexcept;

    Mode  mode  = Mode::Transient;
    Stage stage = Stage::Idle;

    double sampleRate     = 44100.0;
    float  value          = 0.0f;
    float  attackRate     = 0.0f;
    float  decayRate      = 0.0f;
    float  cachedAttack   = -1.0f;
    float  cachedDecay    = -1.0f;

    bool   gateHigh       = false;
    bool   pendingTrigger = false;
    bool   endPulse       = false;
};
