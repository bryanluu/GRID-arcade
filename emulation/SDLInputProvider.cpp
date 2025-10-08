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
            SDL_GameController *p = SDL_GameControllerOpen(i);
            if (p && SDL_GameControllerGetAttached(p) == SDL_TRUE)
            {
                pad = p;
                break;
            }
            if (p)
                SDL_GameControllerClose(p);
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
        case SDL_QUIT:
        {
            if (quitCb)
                quitCb();
            break;
        }
        case SDL_KEYDOWN:
        {
            const SDL_Keycode k = e.key.keysym.sym;
            if (k == SDLK_q || k == SDLK_ESCAPE)
            {
                if (quitCb)
                    quitCb();
            }
            else if (k == SDLK_l)
            {
                if (toggleLEDCb)
                    toggleLEDCb();
            }
            else if (k == SDLK_F1)
            {
                mode = (mode == InputMode::DPad) ? InputMode::Analog : InputMode::DPad;
                if (mode == InputMode::Analog)
                {
                    vx = vy = 0.f;
                    SDL_GetRelativeMouseState(nullptr, nullptr);
                }
            }
            break;
        }
        case SDL_WINDOWEVENT:
        {
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
                lmbHeld = rmbHeld = false;
                vx = vy = 0.f;
            }
            break;
        }
        case SDL_CONTROLLERDEVICEADDED:
        {
            if (!pad)
                openFirstController();
            break;
        }
        case SDL_CONTROLLERDEVICEREMOVED:
        {
            if (pad)
            {
                SDL_Joystick *js = SDL_GameControllerGetJoystick(pad);
                if (js && SDL_JoystickInstanceID(js) == e.cdevice.which)
                {
                    closeController();
                }
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            if (e.button.button == SDL_BUTTON_LEFT)
            {
                lmbHeld = true;
            }
            if (e.button.button == SDL_BUTTON_RIGHT)
            {
                rmbHeld = true;
            }
            break;
        }
        case SDL_MOUSEBUTTONUP:
        {
            if (e.button.button == SDL_BUTTON_LEFT)
            {
                lmbHeld = false;
            }
            if (e.button.button == SDL_BUTTON_RIGHT)
            {
                rmbHeld = false;
            }
            break;
        }
        default:
            break;
        }
    }
}

void SDLInputProvider::sample(InputState &state)
{
    state.pressed = false;

    const millis_t now = SDL_GetTicks();
    const bool padMoving = stickActive(pad);
    const bool analogEngaged = padMoving || lmbHeld;

    // RMB acts as “button” while in Analog
    if (mode == InputMode::Analog || analogEngaged)
    {
        mode = InputMode::Analog;
        lastAnalogActiveMs = now;
        state.pressed = rmbHeld; // Right mouse as digital press in Analog
        genAnalog(state);
    }
    else
    {
        // If previously in Analog, check idle timeout
        if (mode == InputMode::Analog && (now - lastAnalogActiveMs) > analogIdleTimeoutMs)
        {
            mode = InputMode::DPad;
            vx = vy = 0.f; // reset mouse accumulator on exit
        }
        // In D-pad, button is Space (you can also keep RMB if desired)
        state.pressed = (kb && kb[SDL_SCANCODE_SPACE] != 0);
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
        int dx = 0, dy = 0;

        // Only accumulate relative motion when LMB is held
        if (lmbHeld)
        {
            SDL_GetRelativeMouseState(&dx, &dy);
            const float kSens = 0.015f;
            const float decay = 0.12f;
            vx += kSens * float(dx);
            vy += kSens * float(dy);
            // when LMB is down, you can keep weaker decay or none; keep gentle default:
            vx *= (1.f - decay);
            vy *= (1.f - decay);
        }
        else
        {
            // If LMB not held, decay aggressively toward 0 so we quickly “let go”
            vx *= 0.5f;
            vy *= 0.5f;
        }

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
