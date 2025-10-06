#include "Input.h"
#include <algorithm>

// Filters
InputState Input::filterAndClamp(const InputState &s)
{
    InputState o = s;

    // Normalize 0..1023 to -1..+1
    auto toNorm = [](uint16_t adc)
    {
        return (float(adc) / static_cast<float>(InputState::ADC_MAX)) * 2.0f - 1.0f;
    };
    o.x = std::clamp(toNorm(s.x_adc), -1.0f, 1.0f);
    o.y = std::clamp(toNorm(s.y_adc), -1.0f, 1.0f);

    // Deadzone to avoid jitter near center
    auto dead = 0.08f;
    auto dz = [&](float v)
    { return (std::fabs(v) < dead) ? 0.0f : v; };
    o.x = dz(o.x);
    o.y = dz(o.y);

    // Curve for finer control near center
    auto gamma = 1.8f;
    auto curve = [&](float v)
    {
        float a = std::fabs(v);
        return (v >= 0 ? 1.f : -1.f) * std::pow(a, gamma);
    };
    o.x = curve(o.x);
    o.y = curve(o.y);

    // Recompute ADC in case scenes want integers
    auto toAdc = [](float v)
    {
        float f = std::clamp((v + 1.0f) * 511.5f, 0.0f, 1023.0f);
        return static_cast<uint16_t>(std::lround(f));
    };
    o.x_adc = toAdc(o.x);
    o.y_adc = toAdc(o.y);

    return o;
}
