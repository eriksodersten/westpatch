#pragma once

#include <array>
#include <JuceHeader.h>
#include "GroupLayout.h"

class GroupEnvelopeManager
{
public:
    static constexpr int kNumLanes  = GroupLayout::kNumLanes;
    static constexpr int kMaxGroups = GroupLayout::kMaxGroups;

    GroupEnvelopeManager() = default;

    void prepare(double newSampleRate, int maximumBlockSize);
    void reset();

    void setGroupMode(GroupLayout::Mode newMode);
    GroupLayout::Mode getGroupMode() const noexcept;

    void setADSRParameters(const juce::ADSR::Parameters& newParams);
    const juce::ADSR::Parameters& getADSRParameters() const noexcept;

    void noteOnForLane(int lane);
    void noteOffForLane(int lane);

    void allNotesOff();

    // Call once per audio block before lane rendering
    void startBlock();

    // If you want one value per block:
    float getEnvelopeForLane(int lane) const noexcept;
    float getEnvelopeForGroup(int group) const noexcept;

    // If you want one sample at a time:
    float getNextSampleForLane(int lane) noexcept;
    float getNextSampleForGroup(int group) noexcept;

    int getActiveCountForGroup(int group) const noexcept;
    int getNumGroups() const noexcept;

private:
    void applyParametersToAll();
    void resetInactiveGroups();
    bool isValidLane(int lane) const noexcept;
    bool isValidGroup(int group) const noexcept;

    GroupLayout layout;
    juce::ADSR::Parameters adsrParams {};

    std::array<juce::ADSR, kMaxGroups> envelopes;
    std::array<int, kMaxGroups> activeCounts { 0, 0, 0, 0 };
    std::array<float, kMaxGroups> currentValues { 0.0f, 0.0f, 0.0f, 0.0f };

    double sampleRate = 44100.0;
    int maxBlockSize = 512;
};
