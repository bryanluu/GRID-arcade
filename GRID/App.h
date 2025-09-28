#ifndef APP_H
#define APP_H

#include "helpers.h"

#ifdef GRID_EMULATION
    #include "SDLMatrix32.h"
#else
    #include "RGBMatrix32.h"
#endif

class App
{
public:
    Matrix32& m;
    
    App(Matrix32& matrix);
    
    void setup();
    void loop(uint32_t millis_now);
private:
    bool led_mode_{};
};

#endif
