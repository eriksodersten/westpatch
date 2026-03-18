#pragma once

#include <array>
#include <cassert>

class GroupLayout
{
public:
    static constexpr int kNumLanes  = 4;
    static constexpr int kMaxGroups = 4;

    enum class Mode
    {
        Mono, // 1 group:  [0,1,2,3]
        Duo,  // 2 groups: [0,1] [2,3]
        Quad  // 4 groups: [0] [1] [2] [3]
    };

    GroupLayout() = default;
    explicit GroupLayout(Mode newMode) : mode(newMode) {}

    void setMode(Mode newMode) noexcept
    {
        mode = newMode;
    }

    Mode getMode() const noexcept
    {
        return mode;
    }

    int getNumGroups() const noexcept
    {
        switch (mode)
        {
            case Mode::Mono: return 1;
            case Mode::Duo:  return 2;
            case Mode::Quad: return 4;
            default:         return 1;
        }
    }

    int getGroupForLane(int lane) const noexcept
    {
        assert(isValidLane(lane));

        switch (mode)
        {
            case Mode::Mono: return 0;
            case Mode::Duo:  return (lane < 2) ? 0 : 1;
            case Mode::Quad: return lane;
            default:         return 0;
        }
    }

    int getFirstLaneForGroup(int group) const noexcept
    {
        assert(isValidGroupIndex(group));

        switch (mode)
        {
            case Mode::Mono: return 0;
            case Mode::Duo:  return (group == 0) ? 0 : 2;
            case Mode::Quad: return group;
            default:         return 0;
        }
    }

    int getLaneCountForGroup(int group) const noexcept
    {
        assert(isValidGroupIndex(group));

        switch (mode)
        {
            case Mode::Mono: return 4;
            case Mode::Duo:  return 2;
            case Mode::Quad: return 1;
            default:         return 1;
        }
    }

    std::array<int, kNumLanes> getLanesForGroup(int group) const noexcept
    {
        std::array<int, kNumLanes> lanes { -1, -1, -1, -1 };

        const int first = getFirstLaneForGroup(group);
        const int count = getLaneCountForGroup(group);

        for (int i = 0; i < count; ++i)
            lanes[i] = first + i;

        return lanes;
    }

    bool isValidLane(int lane) const noexcept
    {
        return lane >= 0 && lane < kNumLanes;
    }

    bool isValidGroupIndex(int group) const noexcept
    {
        return group >= 0 && group < getNumGroups();
    }

private:
    Mode mode = Mode::Mono;
};
