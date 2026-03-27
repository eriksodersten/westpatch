#include "FunctionGenerator281.h"
#include <JuceHeader.h>

void FunctionGenerator281::prepare (double newSampleRate) noexcept
{
    sampleRate = newSampleRate;
    reset();
}

void FunctionGenerator281::reset() noexcept
{
    stage         = Stage::Idle;
    value         = 0.0f;
    attackRate    = 0.0f;
    decayRate     = 0.0f;
    cachedAttack  = -1.0f;
    cachedDecay   = -1.0f;
    gateHigh      = false;
    pendingTrigger = false;
    endPulse      = false;
}

void FunctionGenerator281::setMode (Mode newMode) noexcept
{
    mode = newMode;
}

void FunctionGenerator281::trigger() noexcept
{
    pendingTrigger = true;
}

void FunctionGenerator281::gate (bool high) noexcept
{
    gateHigh = high;
}

bool FunctionGenerator281::getEndPulse() const noexcept
{
    return endPulse;
}

void FunctionGenerator281::enterAttack() noexcept
{
    stage = Stage::Attack;
}

void FunctionGenerator281::enterDecay() noexcept
{
    stage = Stage::Decay;
}

float FunctionGenerator281::process (float attackTimeSeconds,
                                     float decayTimeSeconds) noexcept
{
    // Cacha rate-beräkning – bara när parametrar ändras
    const float sr = (float) sampleRate;

    if (attackTimeSeconds != cachedAttack)
    {
        cachedAttack = attackTimeSeconds;
        attackRate   = 1.0f / juce::jmax (1.0f, attackTimeSeconds * sr);
    }

    if (decayTimeSeconds != cachedDecay)
    {
        cachedDecay = decayTimeSeconds;
        decayRate   = 1.0f / juce::jmax (1.0f, decayTimeSeconds * sr);
    }

    endPulse = false;

    if (pendingTrigger)
    {
        enterAttack();
        pendingTrigger = false;
    }

    switch (stage)
    {
        case Stage::Idle:
            value = 0.0f;
            break;

        case Stage::Attack:
            value += attackRate;

            if (value >= 1.0f)
            {
                value = 1.0f;

                switch (mode)
                {
                    case Mode::Transient: enterDecay();          break;
                    case Mode::Sustain:   stage = Stage::Sustain; break;
                    case Mode::Cycle:     enterDecay();          break;
                }
            }
            break;

        case Stage::Sustain:
            value = 1.0f;
            if (! gateHigh)
                enterDecay();
            break;

        case Stage::Decay:
            value -= decayRate;

            if (value <= 0.0f)
            {
                value    = 0.0f;
                endPulse = true;

                switch (mode)
                {
                    case Mode::Transient:
                    case Mode::Sustain:
                        stage = Stage::Idle;
                        break;

                    case Mode::Cycle:
                        enterAttack();
                        break;
                }
            }
            break;
    }

    return value;
}
