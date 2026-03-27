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

    const float driven = input * (1.0f + amount * 4.0f);

    const float cell0 =  foldCell (driven,  0.0f,  1.0f) *  0.50f;
    const float cell1 =  foldCell (driven,  1.0f,  0.9f) * -0.40f;
    const float cell2 =  foldCell (driven, -1.0f,  0.9f) * -0.40f;
    const float cell3 =  foldCell (driven,  2.2f,  0.7f) *  0.20f;
    const float cell4 =  foldCell (driven, -2.2f,  0.7f) *  0.20f;
    const float direct = driven * 0.1f;

    return (cell0 + cell1 + cell2 + cell3 + cell4 + direct) * 0.8f;
}
