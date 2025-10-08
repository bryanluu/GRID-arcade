// SDLInputProvider.cpp
#include "SDLInputProvider.h"
#include "Helpers.h"

constexpr float EPSILON = 1e-6f;

bool SDLInputProvider::init(SDL_Window *win)
{
    if (initialized)
        return false; // already initialized

    window = win;
    kb = SDL_GetKeyboardState(nullptr);

    // Prefer gamepad if available
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    openFirstController();

    // Mouse relative + grab as requested
    setMouseRelative(true);

    return true;
}

void SDLInputProvider::shutdown()
{
    closeController();
    setMouseRelative(false);
}

void SDLInputProvider::openFirstController()
{
    if (pad)
        return;
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            pad = SDL_GameControllerOpen(i);
            if (pad)
                break;
        }
    }
}

void SDLInputProvider::closeController()
{
    if (pad)
    {
        SDL_GameControllerClose(pad);
        pad = nullptr;
    }
}

void SDLInputProvider::setMouseRelative(bool enabled)
{
    SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
    SDL_SetWindowGrab(window, enabled ? SDL_TRUE : SDL_FALSE);
}

void SDLInputProvider::pumpEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
            {
                windowFocused = true;
                // Recenter analog accumulator on focus gain
                vx = vy = 0.f;
                SDL_GetRelativeMouseState(nullptr, nullptr); // flush delta
            }
            else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
            {
                windowFocused = false;
                vx = vy = 0.f;
            }
            break;
        case SDL_CONTROLLERDEVICEADDED:
            if (!pad)
                openFirstController(); // auto-open
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (pad && e.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad)))
            {
                closeController();
            }
            break;
        case SDL_KEYDOWN:
            // Example: toggle mode with F1
            if (e.key.keysym.sym == SDLK_F1)
            {
                mode = (mode == InputMode::DPad) ? InputMode::Analog : InputMode::DPad;
                // Reset accumulators when switching into Analog
                if (mode == InputMode::Analog)
                {
                    vx = vy = 0.f;
                    SDL_GetRelativeMouseState(nullptr, nullptr);
                }
            }
            break;
        default:
            break;
        }
    }
}

void SDLInputProvider::sample(InputState &state)
{
    // Button mapping (Space) always available
    state.pressed = kb[SDL_SCANCODE_SPACE];

    // Fallback order: gamepad → mouse → keyboard
    if (mode == InputMode::Analog)
    {
        genAnalog(state);
    }
    else
    {
        genDPad(state);
    }
}

void SDLInputProvider::genDPad(InputState &s)
{
    // Keyboard D-pad only. Ignore ramping for now.
    int x = 0, y = 0;
    if (kb[SDL_SCANCODE_A] || kb[SDL_SCANCODE_LEFT])
        x -= 1;
    if (kb[SDL_SCANCODE_D] || kb[SDL_SCANCODE_RIGHT])
        x += 1;
    if (kb[SDL_SCANCODE_W] || kb[SDL_SCANCODE_UP])
        y -= 1;
    if (kb[SDL_SCANCODE_S] || kb[SDL_SCANCODE_DOWN])
        y += 1;

    // Normalize to unit circle for diagonals
    float fx = float(x), fy = float(y);
    float mag = std::sqrt(fx * fx + fy * fy);
    if (mag > EPSILON)
    {
        fx /= mag;
        fy /= mag;
    }

    if (invertY)
        fy = -fy;

    // Snap to full deflection in ADC space (0 or 1023)
    s.x = fx;
    s.y = fy;
    s.x_adc = toADCFromNorm(s.x);
    s.y_adc = toADCFromNorm(s.y);
}

void SDLInputProvider::genAnalog(InputState &s)
{
    // 1) Try gamepad left stick if present
    bool haveAnalog = false;
    float nx = 0.f, ny = 0.f;

    if (pad)
    {
        // SDL axes are -32768..32767
        const float k = 1.0f / 32767.0f;
        Sint16 ax = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 ay = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);
        nx = Helpers::clamp(ax * k, -1.f, 1.f);
        ny = Helpers::clamp(ay * k, -1.f, 1.f);
        haveAnalog = std::fabs(nx) > 0.001f || std::fabs(ny) > 0.001f;
    }

    // 2) If no gamepad analog, use mouse relative deltas (velocity model)
    if (!haveAnalog && windowFocused)
    {
        int dx, dy;
        Uint32 buttons = SDL_GetRelativeMouseState(&dx, &dy);
        (void)buttons; // button mapped from Space per your spec

        // Sensitivity/decay: you asked to ignore for now; use gentle defaults
        // k: px → norm, decay λ at 60 Hz
        const float kSens = 0.015f;
        const float decay = 0.12f;

        vx += kSens * float(dx);
        vy += kSens * float(dy);

        // Decay to re-center when mouse stops
        vx *= (1.f - decay);
        vy *= (1.f - decay);

        // Clamp to [-1,1]
        nx = Helpers::clamp(vx, -1.f, 1.f);
        ny = Helpers::clamp(vy, -1.f, 1.f);
        haveAnalog = std::fabs(nx) > 0.001f || std::fabs(ny) > 0.001f;
    }

    // 3) If neither provides signal, fall back to keyboard D‑pad
    if (!haveAnalog)
    {
        genDPad(s);
        return;
    }

    // Deadzone and curve using example defaults
    const float dead = 0.08f;
    auto dz = [&](float v)
    { return (std::fabs(v) < dead) ? 0.f : v; };
    nx = dz(nx);
    ny = dz(ny);

    const float gamma = 1.8f;
    auto curve = [&](float v)
    {
        const float a = std::fabs(v);
        return (v >= 0.f ? 1.f : -1.f) * std::pow(a, gamma);
    };
    nx = curve(nx);
    ny = curve(ny);

    if (invertY)
        ny = -ny;

    s.x = Helpers::clamp(nx, -1.f, 1.f);
    s.y = Helpers::clamp(ny, -1.f, 1.f);
    s.x_adc = toADCFromNorm(s.x);
    s.y_adc = toADCFromNorm(s.y);
}
