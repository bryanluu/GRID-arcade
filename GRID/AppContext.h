#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "Matrix32.h"
#include "Timing.h"
#include "Input.h"
#include "Logging.h"
#include "IStorage.h"

// forward decl to avoid header coupling
struct SceneBus;

struct AppContext
{
    Matrix32 &gfx;     // reference to Matrix 32x32 graphics
    Timing &time;      // reference to timing interface
    Input &input;      // reference to input interface
    ILogger &logger;   // reference to logger interface
    IStorage &storage; // reference to storage interface

    // Optional scene router; set by App. May be null in older code.
    SceneBus *bus = nullptr;

    AppContext(Matrix32 &gfx,
               Timing &time,
               Input &input,
               ILogger &logger,
               IStorage &storage)
        : gfx(gfx), time(time), input(input), logger(logger), storage(storage) {}
};

#endif // APP_CONTEXT_H
