#include "Input.h"
#include "Helpers.h"
#include <cmath>
#include <algorithm>

static constexpr float EPSILON = 1e-6f;

// Normalize from calibrated ADC space (0..1023) to -1..+1
void Input::normalizeAndCalibrate(InputState &s)
{
    const float x_adc_min = prov->calib.x_adc_low;
    const float x_adc_center = prov->calib.x_adc_center;
    const float x_adc_max = prov->calib.x_adc_high;
    const float y_adc_min = prov->calib.y_adc_low;
    const float y_adc_center = prov->calib.y_adc_center;
    const float y_adc_max = prov->calib.y_adc_high;

    auto toNorm = [=](float adc, float adc_min, float adc_center, float adc_max)
    {
        // Clamp to calibrated range
        adc = Helpers::clamp(adc, adc_min, adc_max);
        // Map to -1..+1 with center at 0
        if (adc < adc_center)
            return -((adc_center - adc) / std::max(adc_center - adc_min, EPSILON));
        else
            return (adc - adc_center) / std::max(adc_max - adc_center, EPSILON);
    };

    s.x = toNorm(static_cast<float>(s.x_adc), x_adc_min, x_adc_center, x_adc_max);
    s.y = toNorm(static_cast<float>(s.y_adc), y_adc_min, y_adc_center, y_adc_max);
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

InputState Input::processInput(const InputState &s)
{
    InputState o = s;
    normalizeAndCalibrate(o);

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

    return o;
}
