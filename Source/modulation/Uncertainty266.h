#pragma once

class Uncertainty266
{
public:
    struct Params
    {
        float rate = 1.0f;
        float smoothAmount = 0.5f;
        float steppedAmount = 0.5f;
        float density = 0.5f;
        float correlation = 0.5f;
                float spread = 1.0f;
                float slewTime = 0.0f;  // sekunder, 0 = ingen slew
            };

    struct Outputs
    {
        float smooth = 0.0f;
        float stepped = 0.0f;
        float bias = 0.0f;
        float pulse = 0.0f;
    };

    void prepare (double newSampleRate) noexcept;
    void reset() noexcept;

    Outputs process (const Params& params) noexcept;

private:
    double sampleRate = 44100.0;
    float phase = 0.0f;

    float smoothValue = 0.0f;
    float smoothTarget = 0.0f;

    float steppedValue = 0.0f;
        float steppedSlewed = 0.0f;
        float prevSteppedTarget = 0.0f;

        float biasValue = 0.0f;
        float biasTarget = 0.0f;
        float prevSmoothTarget = 0.0f;

    float pulseValue = 0.0f;

    unsigned int rngState = 0x12345678u;

    float nextUniformBipolar() noexcept;
    float nextCenterWeighted() noexcept;
};
