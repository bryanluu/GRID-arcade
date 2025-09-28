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
    Matrix32& matrix;
    
    App(Matrix32& matrix);
    
    void setup();
    void loop(uint32_t millis_now);

    private:
    // Singleton instance of the app
    static App * instance;
    bool led_mode_{};
};

#endif
