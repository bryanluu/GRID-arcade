#ifndef FIXED_STEP_TIMING_H
#define FIXED_STEP_TIMING_H

#include "Timing.h"
#include <SDL.h>
#include <algorithm>

/**
 * @brief A Timing implementation that provides fixed-step timing with cadence control.
 */
class FixedStepTiming final : public Timing
{
    const double defaultTargetHz_{ 60.0 };
    double targetHz_;
    double dtSec_;
    double acc_ = 0.0;
    uint64_t freq_ = 0, last_ = 0;
    uint32_t nowMs_ = 0; // scene clock
    float dtMs_ = 0.0f;
    float fpsEMA_ = 0.0f; // exponential moving average of fps
    static constexpr float k_alpha = 0.2f; // EMA smoothing factor

public:
    explicit FixedStepTiming(double targetHz)
        : defaultTargetHz_(targetHz), targetHz_(targetHz), dtSec_(1.0 / targetHz)
    {
        freq_ = SDL_GetPerformanceFrequency();
        last_ = SDL_GetPerformanceCounter();
        nowMs_ = SDL_GetTicks();
    }

    // Advance wall time; return how many fixed steps to run this frame
    int pump()
    {
        uint64_t now = SDL_GetPerformanceCounter();
        double realDt = double(now - last_) / double(freq_);
        last_ = now;
        if (realDt > 0.25)
            realDt = 0.25; // clamp

        acc_ += realDt;
        int steps = 0;
        while (acc_ >= dtSec_ && steps < 5)
        {
            acc_ -= dtSec_;
            ++steps;
            nowMs_ += static_cast<uint32_t>(dtSec_ * millisPerSec);
            dtMs_ = static_cast<float>(dtSec_ * millisPerSec);
        }
        if (steps > 0)
        {
            float fpsInst = static_cast<float>(targetHz_);
            fpsEMA_ = fpsEMA_ <= 0 ? fpsInst : k_alpha * fpsInst + (1 - k_alpha) * fpsEMA_;
        }
        return steps;
    }

    void sleep_to_cadence()
    {
        double left = dtSec_ - acc_;
        if (left > 0.0)
        {
            double sleepSec = std::max(0.0, left - 0.001);
            sleep(static_cast<Uint32>(sleepSec * millisPerSec));
        }
    }

    // Timing API
    uint32_t nowMs() const override { return nowMs_; }
    float dtMs() const override { return dtMs_; }
    float fps() const override { return fpsEMA_ > 0 ? fpsEMA_ : static_cast<float>(targetHz_); }
    double targetHz() const override { return targetHz_; }

    void setTargetHz(double hz) override
    {
        targetHz_ = BOUND(10.0, hz, 240.0);
        dtSec_ = 1.0 / targetHz_;
        // Optional: smooth retune
    }

    // When there is a preferred timing, apply it; otherwise use default
    void applyPreference(SceneTimingPrefs pref) override
    {
        if (isnan(pref.targetHz))
            setTargetHz(defaultTargetHz_);
        else
            setTargetHz(pref.targetHz);
    }

    void resetSceneClock() override
    {
        nowMs_ = SDL_GetTicks();
        acc_ = 0.0;
        fpsEMA_ = 0.0f;
    }

    void sleep(millis_t ms) override
    {
        SDL_Delay(ms);
    }
};

#endif // FIXED_STEP_TIMING_H
