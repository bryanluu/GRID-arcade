#include "Input.h"
#include "Helpers.h"
#include "IStorage.h"
#include "Logging.h"
#include "Serializer.h"
#include <cmath>
#include <algorithm>

static constexpr float EPSILON = 1e-6f;

size_t InputCalibration::toJSON(char *dst, size_t cap) const
{
    return Serializer::Calibration::toJSON(*this, dst, cap);
}

bool InputCalibration::fromJSON(const char *src)
{
    return Serializer::Calibration::fromJSON(src, *this);
}

bool InputCalibration::save(IStorage &storage, ILogger &log, const char *filename) const
{
    char json[kJsonCap]{};
    const size_t n = toJSON(json, sizeof(json));
    if (n == 0)
    {
        log.logf(LogLevel::Warning, "[%s] toJSON failed", kLogTag);
        return false;
    }
    if (storage.writeAll(filename, json, n))
    {
        log.logf(LogLevel::Debug, "Calibration Saved:\n%s", json);
    }
    else
    {
        log.logf(LogLevel::Warning, "[%s] writeAll failed", kLogTag);
        return false;
    }
    log.logf(LogLevel::Info, "[%s] saved %u bytes to %s", kLogTag, static_cast<unsigned>(n), filename);
    return true;
}

bool InputCalibration::load(IStorage &storage, ILogger &log, const char *filename)
{
    char buf[kReadCap]{};
    const auto r = storage.readAll(filename, buf, sizeof(buf));
    if (!r)
    {
        log.logf(LogLevel::Info, "[%s] %s not found or read failed", kLogTag, filename);
        return false;
    }
    if (fromJSON(buf))
    {
        log.logf(LogLevel::Debug, "Calibration Loaded:\n%s", buf);
    }
    else
    {
        log.logf(LogLevel::Warning, "[%s] fromJSON failed", kLogTag);
        return false;
    }
    log.logf(LogLevel::Info, "[%s] loaded %d bytes from %s", kLogTag, r.bytes, filename);
    return true;
}

bool InputCalibration::load(IStorage &storage, ILogger &log, InputCalibration &out, const char *filename)
{
    return out.load(storage, log, filename);
}

float Input::toNorm(float adc, float adc_min, float adc_center, float adc_max)
{
    // Clamp to calibrated range
    adc = Helpers::clamp(adc, adc_min, adc_max);
    // Map to -1..+1 with center at 0
    if (adc < adc_center)
        return -((adc_center - adc) / std::max(adc_center - adc_min, EPSILON));
    else
        return (adc - adc_center) / std::max(adc_max - adc_center, EPSILON);
};

// Normalize from calibrated ADC space (0..1023) to -1..+1
void Input::normalizeAndCalibrate(InputState &s)
{
    const float x_adc_min = prov->calib.x_adc_low;
    const float x_adc_center = prov->calib.x_adc_center;
    const float x_adc_max = prov->calib.x_adc_high;
    const float y_adc_min = prov->calib.y_adc_low;
    const float y_adc_center = prov->calib.y_adc_center;
    const float y_adc_max = prov->calib.y_adc_high;

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
