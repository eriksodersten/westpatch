#include "GroupEnvelopeManager.h"

void GroupEnvelopeManager::prepare(double newSampleRate, int maximumBlockSize)
{
    sampleRate = newSampleRate;
    maxBlockSize = maximumBlockSize;

    for (auto& env : envelopes)
        env.setSampleRate(sampleRate);

    applyParametersToAll();
    reset();
}

void GroupEnvelopeManager::reset()
{
    for (auto& env : envelopes)
        env.reset();

    activeCounts.fill(0);
    currentValues.fill(0.0f);
}

void GroupEnvelopeManager::setGroupMode(GroupLayout::Mode newMode)
{
    if (layout.getMode() == newMode)
        return;

    layout.setMode(newMode);

    // Safest initial behaviour:
    // changing group mode clears current envelope/gate state.
    resetInactiveGroups();
    allNotesOff();
}

GroupLayout::Mode GroupEnvelopeManager::getGroupMode() const noexcept
{
    return layout.getMode();
}

void GroupEnvelopeManager::setADSRParameters(const juce::ADSR::Parameters& newParams)
{
    adsrParams = newParams;
    applyParametersToAll();
}

const juce::ADSR::Parameters& GroupEnvelopeManager::getADSRParameters() const noexcept
{
    return adsrParams;
}

void GroupEnvelopeManager::noteOnForLane(int lane)
{
    if (! isValidLane(lane))
        return;

    const int group = layout.getGroupForLane(lane);

    if (! isValidGroup(group))
        return;

    const int previousCount = activeCounts[group];
    activeCounts[group] = juce::jmax(0, previousCount + 1);

    if (previousCount == 0)
        envelopes[group].noteOn();
}

void GroupEnvelopeManager::noteOffForLane(int lane)
{
    if (! isValidLane(lane))
        return;

    const int group = layout.getGroupForLane(lane);

    if (! isValidGroup(group))
        return;

    const int previousCount = activeCounts[group];
    activeCounts[group] = juce::jmax(0, previousCount - 1);

    if (previousCount > 0 && activeCounts[group] == 0)
        envelopes[group].noteOff();
}

void GroupEnvelopeManager::allNotesOff()
{
    for (int group = 0; group < kMaxGroups; ++group)
    {
        activeCounts[group] = 0;
        envelopes[group].noteOff();
    }
}

void GroupEnvelopeManager::startBlock()
{
    const int numGroups = layout.getNumGroups();

    for (int group = 0; group < numGroups; ++group)
        currentValues[group] = envelopes[group].getNextSample();

    for (int group = numGroups; group < kMaxGroups; ++group)
        currentValues[group] = 0.0f;
}

float GroupEnvelopeManager::getEnvelopeForLane(int lane) const noexcept
{
    if (! isValidLane(lane))
        return 0.0f;

    return currentValues[layout.getGroupForLane(lane)];
}

float GroupEnvelopeManager::getEnvelopeForGroup(int group) const noexcept
{
    if (! isValidGroup(group))
        return 0.0f;

    return currentValues[group];
}

float GroupEnvelopeManager::getNextSampleForLane(int lane) noexcept
{
    if (! isValidLane(lane))
        return 0.0f;

    return getNextSampleForGroup(layout.getGroupForLane(lane));
}

float GroupEnvelopeManager::getNextSampleForGroup(int group) noexcept
{
    if (! isValidGroup(group))
        return 0.0f;

    const float value = envelopes[group].getNextSample();
    currentValues[group] = value;
    return value;
}

int GroupEnvelopeManager::getActiveCountForGroup(int group) const noexcept
{
    if (group < 0 || group >= kMaxGroups)
        return 0;

    return activeCounts[group];
}

int GroupEnvelopeManager::getNumGroups() const noexcept
{
    return layout.getNumGroups();
}

void GroupEnvelopeManager::applyParametersToAll()
{
    for (auto& env : envelopes)
        env.setParameters(adsrParams);
}

void GroupEnvelopeManager::resetInactiveGroups()
{
    for (int i = 0; i < kMaxGroups; ++i)
    {
        activeCounts[i] = 0;
        currentValues[i] = 0.0f;
        envelopes[i].reset();
    }
}

bool GroupEnvelopeManager::isValidLane(int lane) const noexcept
{
    return lane >= 0 && lane < kNumLanes;
}

bool GroupEnvelopeManager::isValidGroup(int group) const noexcept
{
    return group >= 0 && group < layout.getNumGroups();
}
