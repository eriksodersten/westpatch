#include "Wavefolder.h"
#include <cmath>

float Wavefolder::foldCell (float x, float offset, float scale) noexcept
{
    return std::tanh (x * scale + offset);
}

float Wavefolder::process (float input, float amount) noexcept
{
    if (amount < 0.001f)
        return input;

    const float driven = input * (1.0f + amount * 2.0f);

        const float cell0 = foldCell (driven,  0.00f,  1.6f) *  0.55f;
        const float cell1 = foldCell (driven,  0.90f,  1.3f) * -0.48f;
        const float cell2 = foldCell (driven, -0.70f,  1.1f) * -0.42f;
        const float cell3 = foldCell (driven,  1.90f,  1.2f) *  0.35f;
        const float cell4 = foldCell (driven, -1.60f,  1.0f) *  0.28f;
        const float direct = driven * 0.10f;

        return (cell0 + cell1 + cell2 + cell3 + cell4 + direct) * 0.75f;
}
