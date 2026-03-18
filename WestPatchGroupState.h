#pragma once

struct WestPatchGroupState
{
    bool gate = false;
    int midiNote = -1;
    float frequencyHz = 440.0f;
    int activeNotes = 0;
};
