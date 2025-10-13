#ifndef ARDUINO_PASSIVE_TIMING_H
#define ARDUINO_PASSIVE_TIMING_H

// ArduinoPassiveTiming.h
#include "Timing.h"
#include "Helpers.h"
#include <Arduino.h>

/**
 * @brief A simple Timing implementation for Arduino that just reflects millis()
 * and provides a fixed nominal dt based on targetHz, but does not do any
 * cadence control (no sleeping or frame skipping).
 */
class ArduinoPassiveTiming final : public Timing
{
    const double defaultTargetHz_{60.0};
    double targetHz_;
    float dtMs_;
    millis_t startMs_;

public:
    explicit ArduinoPassiveTiming(double targetHz)
        : defaultTargetHz_(targetHz), targetHz_(targetHz),
          dtMs_(static_cast<float>(MILLIS_PER_SEC / targetHz)),
          startMs_(millis()) {}

    // No cadence control; just reflect Arduino time
    millis_t nowMs() const override { return millis() - startMs_; }
    float dtMs() const override { return dtMs_; } // nominal
    float fps() const override { return static_cast<float>(targetHz_); }
    double targetHz() const override { return targetHz_; }

    // When there is a preferred timing, apply it; otherwise use default
    void applyPreference(SceneTimingPrefs pref) override
    {
        if (isnan(pref.targetHz))
            setTargetHz(defaultTargetHz_);
        else
            setTargetHz(pref.targetHz);
    }

    void setTargetHz(double hz) override
    {
        targetHz_ = hz;
        dtMs_ = static_cast<float>(MILLIS_PER_SEC / targetHz_);
    }
    void resetSceneClock() override
    {
        startMs_ = millis();
    }
    void sleep(millis_t ms) override
    {
        delay(ms);
    }
};

#endif // ARDUINO_PASSIVE_TIMING_H
