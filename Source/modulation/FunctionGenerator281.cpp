#include "FunctionGenerator281.h"

#include <JuceHeader.h>

void FunctionGenerator281::prepare (double newSampleRate) noexcept
{
    sampleRate = newSampleRate;
    reset();
}

void FunctionGenerator281::reset() noexcept
{
    stage = Stage::Idle;
    value = 0.0f;
    pendingTrigger = false;
    cycleMemory = 0.0f;
}

void FunctionGenerator281::setCycle (bool shouldCycle) noexcept
{
    cycle = shouldCycle;
}

void FunctionGenerator281::trigger() noexcept
{
    pendingTrigger = true;
}

void FunctionGenerator281::enterAttack() noexcept
{
    stage = Stage::Attack;

    // Minimal history dependence:
    // täta cykler/retriggers ger lite "spänst", men utan slump.
    cycleMemory = 0.98f * cycleMemory + 0.02f * value;
}

void FunctionGenerator281::enterDecay() noexcept
{
    stage = Stage::Decay;

    // Peak-nivån påverkar nästa cykel mycket subtilt.
    cycleMemory = 0.995f * cycleMemory + 0.005f * value;
}

float FunctionGenerator281::process (float attackTimeSeconds,
                                     float decayTimeSeconds) noexcept
{
    if (pendingTrigger)
    {
        enterAttack();
        pendingTrigger = false;
    }

    switch (stage)
    {
        case Stage::Idle:
        {
            // Långsam återgång mot neutral state
            cycleMemory *= 0.9995f;
            break;
        }

        case Stage::Attack:
        {
            // Mycket liten state-beroende asymmetri, inte random
            const float attackSkew = 1.0f - (cycleMemory * 0.015f);

            const float coeff =
                1.0f / juce::jmax (1.0f,
                                   attackTimeSeconds * attackSkew * (float) sampleRate);

            value += (1.0f - value) * coeff;

            if (value >= 0.999f)
            {
                value = 1.0f;
                enterDecay();
            }

            break;
        }

        case Stage::Decay:
        {
            const float decaySkew = 1.0f + (cycleMemory * 0.02f);

            const float coeff =
                1.0f / juce::jmax (1.0f,
                                   decayTimeSeconds * decaySkew * (float) sampleRate);

            value += (0.0f - value) * coeff;

            if (value <= 0.0005f)
            {
                value = 0.0f;

                if (cycle)
                    enterAttack();
                else
                    stage = Stage::Idle;
            }

            break;
        }
    }

    return value;
}
