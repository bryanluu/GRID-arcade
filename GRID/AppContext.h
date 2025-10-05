#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "Matrix32.h"
#include "Timing.h"

struct AppContext
{
    Matrix32 &gfx; // reference to Matrix 32x32 graphics
    Timing &time;  // reference to timing interface
};

#endif // APP_CONTEXT_H
