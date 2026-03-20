#include "GroupEnvelopeManager.h"
#include <cstddef>

void GroupEnvelopeManager::prepare (double sampleRate)
{
    for (auto& env : envelopes)
        env.setSampleRate (sampleRate);

    reset();
}

void GroupEnvelopeManager::reset()
{
    for (auto& env : envelopes)
        env.reset();

    activeCounts.fill (0);
}

void GroupEnvelopeManager::setNumGroups (int newNumGroups)
{
    const int clamped = juce::jlimit (1, maxGroups, newNumGroups);

    if (clamped == numGroups)
        return;

    numGroups = clamped;
    reset();
}

void GroupEnvelopeManager::setAttackRelease (float attackSeconds, float releaseSeconds)
{
    params.attack  = juce::jmax (0.0001f, attackSeconds);
    params.decay   = 0.0f;
    params.sustain = 1.0f;
    params.release = juce::jmax (0.0001f, releaseSeconds);

    for (auto& env : envelopes)
        env.setParameters (params);
}

void GroupEnvelopeManager::noteOn (int groupIndex) noexcept
{
    if (! isValidGroup (groupIndex))
        return;

    const auto index = static_cast<std::size_t> (groupIndex);

    if (activeCounts[index] == 0)
        envelopes[index].noteOn();

    ++activeCounts[index];
}

void GroupEnvelopeManager::noteOff (int groupIndex) noexcept
{
    if (! isValidGroup (groupIndex))
        return;

    const auto index = static_cast<std::size_t> (groupIndex);

    activeCounts[index] = juce::jmax (0, activeCounts[index] - 1);

    if (activeCounts[index] == 0)
        envelopes[index].noteOff();
}

bool GroupEnvelopeManager::isEnvelopeActive (int groupIndex) const noexcept
{
    if (! isValidGroup (groupIndex))
        return false;

    const auto index = static_cast<std::size_t> (groupIndex);
    return envelopes[index].isActive();
}

void GroupEnvelopeManager::softRetrigger (int groupIndex) noexcept
{
    if (! isValidGroup (groupIndex))
        return;

    const auto index = static_cast<std::size_t> (groupIndex);

    envelopes[index].noteOff();
    envelopes[index].noteOn();
    activeCounts[index] = 1;
}

void GroupEnvelopeManager::hardRetrigger (int groupIndex) noexcept
{
    if (! isValidGroup (groupIndex))
        return;

    const auto index = static_cast<std::size_t> (groupIndex);

    envelopes[index].reset();
    envelopes[index].noteOn();
    activeCounts[index] = 1;
}

float GroupEnvelopeManager::getNextSample (int groupIndex) noexcept
{
    if (! isValidGroup (groupIndex))
        return 0.0f;

    const auto index = static_cast<std::size_t> (groupIndex);
    return envelopes[index].getNextSample();
}

bool GroupEnvelopeManager::isValidGroup (int groupIndex) const noexcept
{
    return groupIndex >= 0 && groupIndex < numGroups;
}
