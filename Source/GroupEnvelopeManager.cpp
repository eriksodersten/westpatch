#include "GroupEnvelopeManager.h"

void GroupEnvelopeManager::prepare(double sampleRate)
{
    for (auto& env : envelopes)
        env.setSampleRate(sampleRate);

    applyParameters();
    reset();
}

void GroupEnvelopeManager::reset()
{
    for (auto& env : envelopes)
        env.reset();

    activeCounts.fill(0);
}

void GroupEnvelopeManager::setNumGroups(int newNumGroups)
{
    const int clamped = juce::jlimit(1, maxGroups, newNumGroups);

    if (clamped == numGroups)
        return;

    numGroups = clamped;
    reset();
}

void GroupEnvelopeManager::setAttackRelease(float attackSeconds, float releaseSeconds)
{
    params.attack = juce::jmax(0.0001f, attackSeconds);
    params.decay = 0.0f;
    params.sustain = 1.0f;
    params.release = juce::jmax(0.0001f, releaseSeconds);

    applyParameters();
}

void GroupEnvelopeManager::noteOn(int groupIndex) noexcept
{
    if (!isValidGroup(groupIndex))
        return;

    if (activeCounts[groupIndex] == 0)
        envelopes[groupIndex].noteOn();

    ++activeCounts[groupIndex];
}

void GroupEnvelopeManager::noteOff(int groupIndex) noexcept
{
    if (!isValidGroup(groupIndex))
        return;

    activeCounts[groupIndex] = juce::jmax(0, activeCounts[groupIndex] - 1);

    if (activeCounts[groupIndex] == 0)
        envelopes[groupIndex].noteOff();
}

float GroupEnvelopeManager::getNextSample(int groupIndex) noexcept
{
    if (!isValidGroup(groupIndex))
        return 0.0f;

    return envelopes[groupIndex].getNextSample();
}

void GroupEnvelopeManager::applyParameters()
{
    for (auto& env : envelopes)
        env.setParameters(params);
}

bool GroupEnvelopeManager::isValidGroup(int groupIndex) const noexcept
{
    return groupIndex >= 0 && groupIndex < numGroups;
}
