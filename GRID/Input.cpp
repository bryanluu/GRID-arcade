#include "Input.h"
#include <algorithm>

// Normalize 0..1023 to -1..+1
void Input::normalize(InputState &s)
{
    auto toNorm = [](uint16_t adc)
    {
        return (float(adc) / static_cast<float>(InputState::ADC_MAX)) * 2.0f - 1.0f;
    };
    s.x = std::clamp(toNorm(s.x_adc), -1.0f, 1.0f);
    s.y = std::clamp(toNorm(s.y_adc), -1.0f, 1.0f);
}

// Apply eadzone to avoid jitter near center
void Input::applyDeadzone(InputState &s)
{
    auto dead = 0.08f;
    auto dz = [&](float v)
    { return (std::fabs(v) < dead) ? 0.0f : v; };
    s.x = dz(s.x);
    s.y = dz(s.y);
}

// Apply curve for finer control near center
void Input::applyCurve(InputState &s)
{
    auto gamma = 1.8f;
    auto curve = [&](float v)
    {
        float a = std::fabs(v);
        return (v >= 0 ? 1.f : -1.f) * std::pow(a, gamma);
    };
    s.x = curve(s.x);
    s.y = curve(s.y);
}

// Recompute ADC in case scenes want integers
void Input::recomputeADC(InputState &s)
{
    auto toAdc = [](float v)
    {
        float f = std::clamp((v + 1.0f) * 511.5f, 0.0f, 1023.0f);
        return static_cast<uint16_t>(std::lround(f));
    };
    s.x_adc = toAdc(s.x);
    s.y_adc = toAdc(s.y);
}

// Filters
InputState Input::processInput(const InputState &s)
{
    InputState o = s;
    normalize(o);
    applyDeadzone(o);
    applyCurve(o);
    recomputeADC(o);

    return o;
}
