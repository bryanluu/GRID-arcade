#ifndef TIMING_H
#define TIMING_H

#include "helpers.h"

// Type
using millis_t = uint32_t; // convenience alias for time in milliseconds

/**
 * @brief Preferences for scene timing
 */
struct SceneTimingPrefs
{
  double targetHz{NAN};
};

struct Timing
{
    static constexpr double millisPerSec = 1000.0;
    virtual ~Timing() = default;
    virtual millis_t nowMs() const = 0;
    virtual float dtMs() const = 0;
    virtual float fps() const = 0;
    virtual double targetHz() const = 0; // use double for stability & precision
    virtual void setTargetHz(double hz) = 0;
    virtual void applyPreference(SceneTimingPrefs pref) = 0;
    virtual void resetSceneClock() = 0;
    virtual void sleep(millis_t ms) = 0;
};

#endif // TIMING_H
