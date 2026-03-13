#pragma once

struct SignalBus
{
    float osc = 0.0f;
    float noise = 0.0f;
    float extIn = 0.0f;

    float env = 0.0f;
    float modEnv = 0.0f;

    float pitchMod = 0.0f;
    float foldMod = 0.0f;
    float lpgCV = 0.0f;

    float synthIn = 0.0f;
    float noiseIn = 0.0f;
    float extScaled = 0.0f;

    float synthFolded = 0.0f;
    float noiseFolded = 0.0f;
    float extFolded = 0.0f;

    float synthBus = 0.0f;

    float lpgOut = 0.0f;
    float extOut = 0.0f;

    float output = 0.0f;
};
