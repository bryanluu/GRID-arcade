#ifndef INPUT_H
#define INPUT_H

#include <cstdint>

using AnalogInput_t = uint16_t;
using DigitalInput_t = bool;

struct InputCalibration
{
    const float deadzone = 0.08f; // 0..1
    const float gamma = 1.8f;     // >0, 1=linear
};

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
};

class IInputProvider
{
    InputCalibration calib;

public:
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
    void normalize(InputState &s);
    void applyDeadzone(InputState &s);
    void applyCurve(InputState &s);
    void recomputeADC(InputState &s);
    InputState processInput(const InputState &s);
};

#endif // INPUT_H
