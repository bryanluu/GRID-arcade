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
    // State

    bool initialized = false;
    millis_t lastAnalogActiveMs = 0;          // time of last analog engagement
    const millis_t analogIdleTimeoutMs = 250; // revert to D-pad after this idle window
    bool lmbHeld = false;
    bool rmbHeld = false;

    SDL_Window *window = nullptr;
    SDL_GameController *pad = nullptr;
    bool windowFocused = true;

    // Config

    InputMode mode = InputMode::DPad; // whether to use D-pad or analog mode
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

public:
    SDLInputProvider(const InputCalibration &c = {}) : IInputProvider(c) {}
    bool init(SDL_Window *win);
    void shutdown();
    ~SDLInputProvider() override { shutdown(); }

    // Call once per frame at 60 Hz
    void pumpEvents();                       // SDL_PollEvent loop, handle focus/mode/buttons, etc.
    void sample(InputState &state) override; // fills state based on current mode and devices

    // Optional: toggle mode externally
    void setMode(InputMode m) { mode = m; }
    InputMode getMode() const { return mode; }

private:
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
    // Check if left stick is moved beyond a small threshold
    static inline bool stickActive(SDL_GameController *pad, float thresh = 0.02f)
    {
        if (!pad)
            return false;
        const float k = 1.0f / 32767.0f;
        float ax = std::abs(SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX) * k);
        float ay = std::abs(SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY) * k);
        return (ax > thresh) || (ay > thresh);
    }
};

#endif // SDL_INPUT_PROVIDER_H
