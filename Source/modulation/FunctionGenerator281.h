#pragma once

class FunctionGenerator281
{
public:
    void prepare (double newSampleRate) noexcept;
    void reset() noexcept;

    void setCycle (bool shouldCycle) noexcept;
    void trigger() noexcept;

    float process (float attackTimeSeconds,
                   float decayTimeSeconds) noexcept;

private:
    enum class Stage
    {
        Idle,
        Attack,
        Decay
    };

    void enterAttack() noexcept;
    void enterDecay() noexcept;

    Stage stage = Stage::Idle;

    double sampleRate = 44100.0;
    float value = 0.0f;

    bool cycle = false;
    bool pendingTrigger = false;

    // Subtil history dependence, ej random
    float cycleMemory = 0.0f;
};
