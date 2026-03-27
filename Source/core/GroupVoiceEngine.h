#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "dsp/WestPatchLane.h"

namespace westpatch::engine
{

static constexpr int kNumLanesPerGroup = 4;
static constexpr int kMaxGroupVoices = 4;
static constexpr int kMaxTailVoices = 4;
static constexpr int kDefaultTailReleaseSamples = 64;

enum class ActiveGroupMode
{
    Unison = 1,
    Duo = 2,
    Quad = 4
};

enum class NoteOnAction
{
    StartIdleGroup,
    ReuseReleasingGroup,
    StealActiveGroup
};

struct GlobalModSnapshot
{
    float pitchModSemitones = 0.0f;
    float foldModAmount = 0.0f;
    float lpgCvMod = 0.0f;
};

struct GroupVoiceState
{
    bool active = false;
    bool gate = false;
    bool envelopeActive = false;
    int midiNote = -1;
    float frequencyHz = 440.0f;
    std::uint64_t allocationSerial = 0;
    std::array<WestPatchLane, kNumLanesPerGroup> lanes {};
};

struct TailVoiceState
{
    bool active = false;
    int sourceGroupIndex = -1;
    int samplesRemaining = 0;
    float frequencyHz = 440.0f;
    float gain = 0.0f;
    float gainStep = 0.0f;
    GlobalModSnapshot mod {};
    std::array<WestPatchLane, kNumLanesPerGroup> lanes {};
};

struct NoteOnDecision
{
    int groupIndex = 0;
    NoteOnAction action = NoteOnAction::StartIdleGroup;
};

class GroupVoiceEngine
{
public:
    void reset() noexcept
    {
        activeGroupCount_ = 1;
        nextAllocationSerial_ = 1;

        for (auto& group : groups_)
            group = {};

        for (auto& tail : tails_)
            tail = {};
    }

    void setActiveGroupCount (int count) noexcept
    {
        if (count < 1)
            count = 1;

        if (count > kMaxGroupVoices)
            count = kMaxGroupVoices;

        activeGroupCount_ = count;
    }

    int getActiveGroupCount() const noexcept
    {
        return activeGroupCount_;
    }

    std::uint64_t claimAllocationSerial() noexcept
    {
        return nextAllocationSerial_++;
    }

    GroupVoiceState& group (int index) noexcept
    {
        return groups_[static_cast<std::size_t> (index)];
    }

    const GroupVoiceState& group (int index) const noexcept
    {
        return groups_[static_cast<std::size_t> (index)];
    }

    TailVoiceState& tail (int index) noexcept
    {
        return tails_[static_cast<std::size_t> (index)];
    }

    const TailVoiceState& tail (int index) const noexcept
    {
        return tails_[static_cast<std::size_t> (index)];
    }

    NoteOnDecision findGroupForNoteOn() const noexcept
    {
        const int numGroups = activeGroupCount_;

        for (int g = 0; g < numGroups; ++g)
        {
            const auto& voice = groups_[static_cast<std::size_t> (g)];

            if (! voice.gate && ! voice.envelopeActive)
                return { g, NoteOnAction::StartIdleGroup };
        }

        int oldestReleasingGroup = -1;
        std::uint64_t oldestReleasingSerial = 0;

        for (int g = 0; g < numGroups; ++g)
        {
            const auto& voice = groups_[static_cast<std::size_t> (g)];

            if (! voice.gate && voice.envelopeActive)
            {
                if (oldestReleasingGroup < 0 || voice.allocationSerial < oldestReleasingSerial)
                {
                    oldestReleasingGroup = g;
                    oldestReleasingSerial = voice.allocationSerial;
                }
            }
        }

        if (oldestReleasingGroup >= 0)
            return { oldestReleasingGroup, NoteOnAction::ReuseReleasingGroup };

        int oldestActiveGroup = 0;
        std::uint64_t oldestActiveSerial = groups_[0].allocationSerial;

        for (int g = 1; g < numGroups; ++g)
        {
            const auto& voice = groups_[static_cast<std::size_t> (g)];

            if (voice.allocationSerial < oldestActiveSerial)
            {
                oldestActiveSerial = voice.allocationSerial;
                oldestActiveGroup = g;
            }
        }

        return { oldestActiveGroup, NoteOnAction::StealActiveGroup };
    }

    int findGroupForNoteOff (int midiNoteNumber) const noexcept
    {
        int bestGroup = -1;
        std::uint64_t newestSerial = 0;

        for (int g = 0; g < activeGroupCount_; ++g)
        {
            const auto& voice = groups_[static_cast<std::size_t> (g)];

            if (voice.gate && voice.midiNote == midiNoteNumber)
            {
                if (bestGroup < 0 || voice.allocationSerial > newestSerial)
                {
                    bestGroup = g;
                    newestSerial = voice.allocationSerial;
                }
            }
        }

        return bestGroup;
    }

private:
    int activeGroupCount_ = 1;
    std::uint64_t nextAllocationSerial_ = 1;
    std::array<GroupVoiceState, kMaxGroupVoices> groups_ {};
    std::array<TailVoiceState, kMaxTailVoices> tails_ {};
};

} // namespace westpatch::engine

