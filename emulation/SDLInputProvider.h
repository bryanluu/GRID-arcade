#ifndef SDL_INPUT_PROVIDER_H
#define SDL_INPUT_PROVIDER_H

// SDLInputProvider.h
#include "Input.h"
#include "Helpers.h"
#include <functional>
#include <SDL.h>

// High-level input mode
enum class InputMode
{
    DPad,
    Analog
};

// Centralized tuning constants (replaces magic numbers)
struct InputTuning
{
    // Float epsilon for near-zero checks and normalization guards.
    static constexpr float EPSILON = 1e-6f; // was 1e-6f

    // Minimal stick magnitude to treat gamepad as "active" (filters noise).
    static constexpr float PAD_MOVEMENT_THRESHOLD = 0.005f; // was 0.005f

    // Time window (ms) to keep Analog engaged after last analog activity.
    static constexpr millis_t ANALOG_IDLE_TIMEOUT_MS = 250; // was 250

    // SDL controller axis scaling: SDL reports [-32768..32767]; map to [-1..1].
    // Use 32767 so +32767 → +1.0 exactly. −32768 slightly underflows and is clamped.
    static constexpr float SDL_AXIS_SCALE = 1.0f / 32767.0f; // was 1.0/32767.0

    // Mouse “velocity” analog model sensitivity and decay (per ~60 Hz tick).
    static constexpr float MOUSE_SENS = 0.015f; // was 0.015f
    static constexpr float MOUSE_DECAY = 0.12f; // was 0.12f

    // Faster decay when LMB not held to quickly relinquish analog.
    static constexpr float MOUSE_FAST_DECAY = 0.5f; // was 0.5f
};

class SDLInputProvider final : public IInputProvider
{
    // State
    bool initialized = false;
    // Time of last analog engagement (mouse LMB held or gamepad moved)
    millis_t lastAnalogActiveMs = 0;
    // Revert to D‑pad after this idle window
    static constexpr millis_t analogIdleTimeoutMs = InputTuning::ANALOG_IDLE_TIMEOUT_MS;

    bool lmbHeld = false;
    bool rmbHeld = false;

    SDL_Window *window = nullptr;
    SDL_GameController *pad = nullptr;
    bool windowFocused = true;

    // Callbacks
    std::function<void()> quitCb;      // Q or Esc
    std::function<void()> toggleLEDCb; // L
    std::function<void()> resizeCb;    // resize window

    // Config
    InputMode mode = InputMode::DPad; // whether to use D‑pad or analog mode
    bool invertY = false;

    // Keyboard state cache
    const uint8_t *kb = nullptr;

    // Mouse analog accumulator (velocity model)
    float vx = 0.f, vy = 0.f;

public:
    // Experimentally determined defaults (documented)
    // deadzone: ignore |v| < 0.02; gamma: response curve exponent
    static constexpr InputCalibration defaultCalib{
        0.02f,                                                       // deadzone
        1.8f,                                                        // gamma
        InputCalibration::ADC_MIN,                                   // x_adc_low
        (InputCalibration::ADC_MAX - InputCalibration::ADC_MIN) / 2, // x_adc_center
        InputCalibration::ADC_MAX,                                   // x_adc_high
        InputCalibration::ADC_MIN,                                   // y_adc_low
        (InputCalibration::ADC_MAX - InputCalibration::ADC_MIN) / 2, // y_adc_center
        InputCalibration::ADC_MAX,                                   // y_adc_high
    };

    SDLInputProvider(const InputCalibration &c = defaultCalib) : IInputProvider(c) {}
    bool init(SDL_Window *win);
    void shutdown();
    ~SDLInputProvider() override { shutdown(); }

    // Single SDL_PollEvent loop, handles focus/mode/buttons
    void pumpEvents();

    // Hooks
    void onQuit(std::function<void()> cb) { quitCb = std::move(cb); }
    void onToggleLED(std::function<void()> cb) { toggleLEDCb = std::move(cb); }
    void onResize(std::function<void()> cb) { resizeCb = std::move(cb); }

    // Fills state based on current mode and devices
    void sample(InputState &state) override;

    // Optional: toggle mode externally
    void setMode(InputMode m) { mode = m; }
    InputMode getMode() const { return mode; }

private:
    // Aliases for brevity inside methods
    static constexpr float EPSILON = InputTuning::EPSILON;
    static constexpr float PAD_MOVEMENT_THRESHOLD = InputTuning::PAD_MOVEMENT_THRESHOLD;

    // Helpers
    void openFirstController();
    void closeController();
    void setMouseRelative(bool enabled);

    void clampMagnitudeToOne(float &x, float &y);
    void normalizeToUnitCircle(float &x, float &y);
    void useSDLAxis(float &nx, float &ny, bool &haveAnalog);
    void useMouseDeltas(float &nx, float &ny, bool &haveAnalog);

    // Generators
    void genDPad(InputState &s);
    void genAnalog(InputState &s);

    // Event handlers
    void handleKeyDown(const SDL_KeyboardEvent &key);
    void handleWindowEvent(const SDL_WindowEvent &we);
    void handleMouseButtonDown(const SDL_MouseButtonEvent &be);
    void handleMouseButtonUp(const SDL_MouseButtonEvent &be);

    // Utilities
    static inline float toNormFromADC(AnalogInput_t adc)
    {
        return ((float(adc) / float(InputCalibration::ADC_MAX)) * 2.f) - 1.f;
    }
    static inline AnalogInput_t toADCFromNorm(float v)
    {
        const float f = Helpers::clamp((v + 1.f) * (InputCalibration::ADC_MAX / 2.f),
                                       0.f, float(InputCalibration::ADC_MAX));
        return static_cast<AnalogInput_t>(lroundf(f));
    }

    // Check if left stick is moved beyond a small threshold
    static inline bool stickActive(SDL_GameController *pad,
                                   float thresh = SDLInputProvider::PAD_MOVEMENT_THRESHOLD)
    {
        if (!pad)
            return false;
        // Scale SDL [-32768..32767] into [-1..1]; choose 32767 so +max maps to +1.0
        const float k = InputTuning::SDL_AXIS_SCALE;
        float ax = std::abs(SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX) * k);
        float ay = std::abs(SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY) * k);
        return (ax > thresh) || (ay > thresh);
    }
};

#endif // SDL_INPUT_PROVIDER_H
