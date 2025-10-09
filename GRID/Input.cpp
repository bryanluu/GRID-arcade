#include "Input.h"
#include "Helpers.h"
#include <cmath>
#include <algorithm>

// Normalize 0..1023 to -1..+1
float Input::normalize(AnalogInput_t adc)
{
    return ((float(adc) / static_cast<float>(InputState::ADC_MAX)) * 2.0f) - 1.0f;
}

// Apply deadzone to avoid jitter near center
// v in -1..+1, deadzone in 0..1
float Input::applyDeadzone(float v)
{
    auto dead = prov->calib.deadzone;
    return (std::fabs(v) <= dead) ? 0.0f : v;
}

// Apply curve for finer control near center
// v' = sign(v) * |v|^γ
// Gamma usually between 1.5 and 2.5
// Higher gamma means more curve, less sensitivity near center
// v in -1..+1
float Input::applyCurve(float v)
{
    auto gamma = prov->calib.gamma;
    float a = std::fabs(v);
    return (v >= 0 ? 1.f : -1.f) * std::pow(a, gamma);
}

// Recompute ADC in case scenes want integers
void Input::recomputeADC(InputState &s)
{
    auto toAdc = [](float v)
    {
        float f = Helpers::clamp((v + 1.0f) * 511.5f, 0.0f, 1023.0f);
        return static_cast<AnalogInput_t>(std::lround(f));
    };
    s.x_adc = toAdc(s.x);
    s.y_adc = toAdc(s.y);
}

InputState Input::processInput(const InputState &s)
{
    static const float EPSILON = 1e-6f;
    InputState o = s;
    o.x = normalize(o.x_adc);
    o.y = normalize(o.y_adc);

    // Convert polar, apply deadzone and curve radially
    Vector v = o.vec();
    float r = length(v);
    float rp = applyDeadzone(r);
    rp = applyCurve(rp);
    rp = Helpers::clamp(rp, 0.0f, 1.0f);

    // Rescale vector to new magnitude
    if (r > EPSILON)
    {
        o.x = (v.x / r) * rp;
        o.y = (v.y / r) * rp;
    }
    else
    {
        o.x = 0.0f;
        o.y = 0.0f;
    }
    o.x = Helpers::clamp(o.x, -1.0f, +1.0f);
    o.y = Helpers::clamp(o.y, -1.0f, +1.0f);
    recomputeADC(o);

    return o;
}
