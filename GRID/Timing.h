#ifndef TIMING_H
#define TIMING_H

#include "helpers.h"

// Type
using millis_t = uint32_t; // convenience alias for time in milliseconds

struct Timing
{
    virtual ~Timing() = default;
    virtual millis_t nowMs() const = 0;
    virtual float dtMs() const = 0;
    virtual float fps() const = 0;
    virtual double targetHz() const = 0; // use double for stability & precision
    virtual void setTargetHz(double hz) = 0;
    virtual void resetSceneClock() = 0;
    virtual void delay(millis_t ms) { delay(ms); } // default implementation
};

#endif // TIMING_H
