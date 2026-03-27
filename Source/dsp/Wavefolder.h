#pragma once

class Wavefolder
{
public:
    float process (float input, float amount) noexcept;

private:
    static float foldCell (float x, float offset, float scale) noexcept;
};
