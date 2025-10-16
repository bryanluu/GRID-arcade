// SDLInputProvider.cpp
#include "SDLInputProvider.h"
#include "Helpers.h"

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

    initialized = true;
    return true;
}

void SDLInputProvider::shutdown()
{
    closeController();
    setMouseRelative(false);
    initialized = false;
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

void SDLInputProvider::handleKeyDown(const SDL_KeyboardEvent &key)
{
    const SDL_Keycode k = key.keysym.sym;
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
}

void SDLInputProvider::handleWindowEvent(const SDL_WindowEvent &we)
{
    if (we.event == SDL_WINDOWEVENT_FOCUS_GAINED)
    {
        windowFocused = true;
        // Recenter analog accumulator on focus gain
        vx = vy = 0.f;
        SDL_GetRelativeMouseState(nullptr, nullptr); // flush delta
    }
    else if (we.event == SDL_WINDOWEVENT_FOCUS_LOST)
    {
        windowFocused = false;
        lmbHeld = rmbHeld = false;
        vx = vy = 0.f;
    }
}

void SDLInputProvider::handleMouseButtonDown(const SDL_MouseButtonEvent &be)
{
    if (be.button == SDL_BUTTON_LEFT)
        lmbHeld = true;
    if (be.button == SDL_BUTTON_RIGHT)
        rmbHeld = true;
}

void SDLInputProvider::handleMouseButtonUp(const SDL_MouseButtonEvent &be)
{
    if (be.button == SDL_BUTTON_LEFT)
        lmbHeld = false;
    if (be.button == SDL_BUTTON_RIGHT)
        rmbHeld = false;
}

void SDLInputProvider::pumpEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            if (quitCb)
                quitCb();
            break;
        case SDL_KEYDOWN:
            handleKeyDown(e.key);
            break;
        case SDL_WINDOWEVENT:
            handleWindowEvent(e.window);
            break;
        case SDL_CONTROLLERDEVICEADDED:
            if (!pad)
                openFirstController();
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (pad)
            {
                SDL_Joystick *js = SDL_GameControllerGetJoystick(pad);
                if (js && SDL_JoystickInstanceID(js) == e.cdevice.which)
                {
                    closeController();
                }
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            handleMouseButtonDown(e.button);
            break;
        case SDL_MOUSEBUTTONUP:
            handleMouseButtonUp(e.button);
            break;
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

    // While in Analog, RMB acts as “button”; otherwise Space in D‑pad.
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
        if (mode == InputMode::Analog &&
            (now - lastAnalogActiveMs) > analogIdleTimeoutMs)
        {
            mode = InputMode::DPad;
            vx = vy = 0.f; // reset mouse accumulator on exit
        }
        // In D‑pad, button is Space
        state.pressed = (kb && kb[SDL_SCANCODE_SPACE] != 0);
        genDPad(state);
    }
}

void SDLInputProvider::clampMagnitudeToOne(float &x, float &y)
{
    float m = std::sqrt(x * x + y * y);
    if (m > 1.f && m > InputTuning::EPSILON)
    {
        x /= m;
        y /= m;
    }
}

void SDLInputProvider::normalizeToUnitCircle(float &x, float &y)
{
    float mag = std::sqrt(x * x + y * y);
    if (mag > InputTuning::EPSILON)
    {
        x /= mag;
        y /= mag;
    }
    else
    {
        x = 0.f;
        y = 0.f;
    }
}

void SDLInputProvider::genDPad(InputState &s)
{
    // Keyboard D‑pad only. Ignore ramping for now.
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
    normalizeToUnitCircle(fx, fy);

    if (invertY) // typical camera-style invert
        fy = -fy;

    // Snap to full deflection in ADC space (0 or 1023)
    s.x = fx;
    s.y = fy;
    s.x_adc = toADCFromNorm(s.x);
    s.y_adc = toADCFromNorm(s.y);
}

void SDLInputProvider::useSDLAxis(float &x, float &y, bool &haveAnalog)
{
    if (pad)
    {
        // SDL axes are −32768..32767 → scale to [−1..1]
        const float k = InputTuning::SDL_AXIS_SCALE;
        Sint16 ax = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 ay = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);
        x = Helpers::clamp(ax * k, -1.f, 1.f);
        y = Helpers::clamp(ay * k, -1.f, 1.f);
        haveAnalog = std::fabs(x) > InputTuning::EPSILON ||
                     std::fabs(y) > InputTuning::EPSILON;
    }
}

void SDLInputProvider::useMouseDeltas(float &nx, float &ny, bool &haveAnalog)
{
    int dx = 0, dy = 0;

    // Only accumulate relative motion when LMB is held
    if (lmbHeld)
    {
        SDL_GetRelativeMouseState(&dx, &dy);
        // Velocity model: accumulate scaled deltas then decay toward center each tick.
        const float kSens = InputTuning::MOUSE_SENS;  // sensitivity px→norm
        const float decay = InputTuning::MOUSE_DECAY; // per‑tick decay (~60 Hz)
        vx += kSens * float(dx);
        vy += kSens * float(dy);
        // gentle decay while held
        vx *= (1.f - decay);
        vy *= (1.f - decay);
    }
    else
    {
        // Fast decay when not held to quickly “let go”
        vx *= InputTuning::MOUSE_FAST_DECAY;
        vy *= InputTuning::MOUSE_FAST_DECAY;
    }

    nx = Helpers::clamp(vx, -1.f, 1.f);
    ny = Helpers::clamp(vy, -1.f, 1.f);
    haveAnalog = std::fabs(nx) > InputTuning::EPSILON ||
                 std::fabs(ny) > InputTuning::EPSILON;
}

void SDLInputProvider::genAnalog(InputState &s)
{
    // 1) Try gamepad left stick if present
    bool haveAnalog = false;
    float nx = 0.f, ny = 0.f;

    if (pad)
        useSDLAxis(nx, ny, haveAnalog);

    // 2) If no gamepad analog, use mouse relative deltas (velocity model)
    if (!haveAnalog && windowFocused)
    {
        useMouseDeltas(nx, ny, haveAnalog);
    }

    // 3) If neither provides signal, fall back to keyboard D‑pad
    if (!haveAnalog)
    {
        genDPad(s);
        return;
    }

    clampMagnitudeToOne(s.x, s.y);

    if (invertY) // typical camera-style invert
        ny = -ny;

    s.x = Helpers::clamp(nx, -1.f, 1.f);
    s.y = Helpers::clamp(ny, -1.f, 1.f);
    s.x_adc = toADCFromNorm(s.x);
    s.y_adc = toADCFromNorm(s.y);
}
