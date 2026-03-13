#include "Wavefolder.h"

float Wavefolder::process (float input, float amount) const noexcept
{
    float x = input * amount;

    while (x > 1.0f || x < -1.0f)
    {
        if (x > 1.0f)
            x = 2.0f - x;
        else if (x < -1.0f)
            x = -2.0f - x;
    }

    return x;
}
