#ifndef SDL_INPUT_PROVIDER_H
#define SDL_INPUT_PROVIDER_H

// SDLInputProvider.h
#include "Input.h"
#include "Helpers.h"
#include <SDL.h>

enum class InputMode
{
    DPad,
    Analog
};

class SDLInputProvider final : public IInputProvider
{
    bool initialized = false;

public:
    SDLInputProvider(const InputCalibration &c = {}) : IInputProvider(c) {}
    bool init(SDL_Window *win);
    void shutdown();
    ~SDLInputProvider() override { shutdown(); }

    // Call once per frame at 60 Hz
    void pumpEvents();              // SDL_PollEvent loop, handle focus/mode/buttons, etc.
    void sample(InputState &state); // fills state based on current mode and devices

    // Optional: toggle mode externally
    void setMode(InputMode m) { mode = m; }
    InputMode getMode() const { return mode; }

private:
    // State
    SDL_Window *window = nullptr;
    SDL_GameController *pad = nullptr;
    bool windowFocused = true;

    // Config
    InputMode mode = InputMode::DPad; // default
    bool invertY = false;

    // Keyboard state cache
    const uint8_t *kb = nullptr;

    // Mouse analog accumulator (velocity model)
    float vx = 0.f, vy = 0.f;

    // Helpers
    void openFirstController();
    void closeController();
    void setMouseRelative(bool enabled);

    // Generators
    void genDPad(InputState &s);
    void genAnalog(InputState &s);

    // Utilities
    static inline float toNormFromADC(AnalogInput_t adc)
    {
        return ((float(adc) / float(InputState::ADC_MAX)) * 2.f) - 1.f;
    }
    static inline AnalogInput_t toADCFromNorm(float v)
    {
        const float f = Helpers::clamp((v + 1.f) * (InputState::ADC_MAX / 2.f), 0.f, float(InputState::ADC_MAX));
        return static_cast<AnalogInput_t>(lroundf(f));
    }
};

#endif // SDL_INPUT_PROVIDER_H
