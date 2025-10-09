#ifndef INPUT_H
#define INPUT_H

#include "Vector.h"
#include <cstdint>

using AnalogInput_t = uint16_t;
using DigitalInput_t = bool;

struct InputState
{
    constexpr static int ADC_MAX = 1023;

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
    const float deadzone = 0.08f; // 0..1
    const float gamma = 1.8f;     // >0, 1=linear
    // ADC calibration (if needed)
    const AnalogInput_t x_adc_low = 0;
    const AnalogInput_t x_adc_center = InputState::ADC_MAX / 2; // usually 512
    const AnalogInput_t x_adc_high = InputState::ADC_MAX;
    const AnalogInput_t y_adc_low = 0;
    const AnalogInput_t y_adc_center = InputState::ADC_MAX / 2;
    const AnalogInput_t y_adc_high = InputState::ADC_MAX;

    constexpr InputCalibration() = default;
};

class IInputProvider
{
public:
    const InputCalibration calib;
    IInputProvider(const InputCalibration &c) : calib(c) {}
    virtual ~IInputProvider() = default;
    // Called once per tick; must be non-blocking
    virtual void sample(InputState &out) = 0;
};

class Input
{
public:
    void init(IInputProvider *provider) { prov = provider; }
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
