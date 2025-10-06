#ifndef INPUT_H
#define INPUT_H

#include <cstdint>

struct InputState
{
    constexpr static int ADC_MAX = 1023;

    // Raw Arduino-like
    uint16_t x_adc; // 0..1023
    uint16_t y_adc; // 0..1023
    bool pressed;

    // Normalized helpers
    float x; // -1..+1
    float y; // -1..+1
};

class IInputProvider
{
public:
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
        current = filterAndClamp(raw);
        frameId++;
    }
    const InputState &state() const { return current; }
    uint64_t frame() const { return frameId; }

private:
    IInputProvider *prov = nullptr;
    InputState current{};
    uint64_t frameId = 0;
    InputState filterAndClamp(const InputState &s);
};

#endif // INPUT_H
