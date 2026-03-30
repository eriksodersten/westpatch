#pragma once

#include <JuceHeader.h>

struct WestPatchGroupState
{
    bool gate = false;
    int midiNote = -1;
    float frequencyHz = 440.0f;

    juce::SmoothedValue<float> glideSmoothed { 440.0f };
};
