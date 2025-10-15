#ifndef INPUT_H
#define INPUT_H

#include "Serializer.h"
#include "Vector.h"
#include <cstdint>

using AnalogInput_t = uint16_t;
using DigitalInput_t = bool;

// forward decl
struct IStorage;
struct ILogger;

struct InputState
{
    // Raw Arduino-like
    AnalogInput_t x_adc; // 0..1023
    AnalogInput_t y_adc; // 0..1023
    DigitalInput_t pressed;

    // Normalized helpers
    float x; // -1..+1
    float y; // -1..+1

    // Vector form of (x,y)
    Vector vec() const { return Vector{x, y}; }
};

struct InputCalibration
{
    // ---- Persistence config ----
    static constexpr const char *kDefaultFile = "calibration.json"; // default save path (relative to storage base)
    static constexpr size_t kJsonCap = 192;                         // serialize buffer capacity
    static constexpr size_t kReadCap = 256;                         // read buffer capacity (>= kJsonCap)
    static constexpr const char *kLogTag = "Calib";

    // ---- Data ----
    float deadzone = 0.08f; // 0..1
    float gamma = 1.8f;     // >0, 1=linear
    // ADC calibration (if needed)
    static constexpr int ADC_MIN = 0;
    static constexpr int ADC_MAX = 1023;
    AnalogInput_t x_adc_low = ADC_MIN;
    AnalogInput_t x_adc_center = ADC_MAX / 2; // usually 512
    AnalogInput_t x_adc_high = ADC_MAX;
    AnalogInput_t y_adc_low = ADC_MIN;
    AnalogInput_t y_adc_center = ADC_MAX / 2;
    AnalogInput_t y_adc_high = ADC_MAX;

    constexpr InputCalibration() = default;
    constexpr InputCalibration(float dz, float gm,
                               uint16_t xl, uint16_t xc, uint16_t xh,
                               uint16_t yl, uint16_t yc, uint16_t yh)
        : deadzone(dz), gamma(gm),
          x_adc_low(xl), x_adc_center(xc), x_adc_high(xh),
          y_adc_low(yl), y_adc_center(yc), y_adc_high(yh) {}

    // Serialize this object to JSON into dst.
    // Returns bytes written, or 0 on failure.
    size_t toJSON(char *dst, size_t cap) const;

    // Parse JSON into this object. Returns true on success.
    bool fromJSON(const char *src);

    // Save/load using IStorage (atomic write) with default filename.
    bool save(IStorage &storage, ILogger &log, const char *filename = "calibration.json") const;
    bool load(IStorage &storage, ILogger &log, const char *filename = "calibration.json");

    // Optional: static factory that returns a loaded instance.
    static bool load(IStorage &storage, ILogger &log, InputCalibration &out, const char *filename = "calibration.json");
};

class IInputProvider
{
public:
    InputCalibration calib;
    IInputProvider(const InputCalibration &c) : calib(c) {}
    virtual ~IInputProvider() = default;
    // Called once per tick; must be non-blocking
    virtual void sample(InputState &out) = 0;
};

class Input
{
public:
    void init(IInputProvider *provider)
    {
        prov = provider;
    }
    // Call at start of each tick
    void sample()
    {
        InputState raw;
        prov->sample(raw);
        current = processInput(raw);
        frameId++;
    }
    const InputState &state() const { return current; }
    uint64_t frame() const { return frameId; }

    const InputCalibration &getCalibration() const { return prov->calib; };
    void setCalibration(InputCalibration &calibration) { prov->calib = calibration; }
    static float toNorm(float adc, float adc_min, float adc_center, float adc_max);

private:
    IInputProvider *prov = nullptr;
    InputState current{};
    uint64_t frameId = 0;
    void normalizeAndCalibrate(InputState &s);
    float applyDeadzone(float v);
    float applyCurve(float v);
    void recomputeADC(InputState &s);
    InputState processInput(const InputState &s);
};

#endif // INPUT_H
