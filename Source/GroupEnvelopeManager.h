#pragma once

#include <JuceHeader.h>
#include <array>

class GroupEnvelopeManager
{
public:
    static constexpr int maxGroups = 4;

    void prepare(double sampleRate);
    void reset();
    void setNumGroups(int newNumGroups);
    int getNumGroups() const noexcept { return numGroups; }

    void setAttackRelease(float attackSeconds, float releaseSeconds);

    void noteOn(int groupIndex) noexcept;
    void noteOff(int groupIndex) noexcept;
    bool isEnvelopeActive(int groupIndex) const noexcept;
    float getNextSample(int groupIndex) noexcept;

private:
    void applyParameters();
    bool isValidGroup(int groupIndex) const noexcept;

    std::array<juce::ADSR, maxGroups> envelopes;
    juce::ADSR::Parameters params {};
    int numGroups = 1;
    std::array<int, maxGroups> activeCounts { 0, 0, 0, 0 };
};
