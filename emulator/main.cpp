#include "GRID_App.h"
#include "SDLMatrix32.h"
#include "Matrix32Adapter.h"
#include <SDL.h>
#include <cstdint>

static uint32_t millis_now(uint32_t start) { return SDL_GetTicks() - start; }

void setup(SDLMatrix32 &m)
{
	Matrix32Adapter gfx(m);
	gfx.fillRect(0,0,32,32, rgb(0,128,0));
	gfx.drawRect(0,0,32,32, rgb(255,255,0));
    gfx.drawPixel(31, 31, rgb(0, 0, 255));
	gfx.setCursor(1,0);
	gfx.setTextSize(1);
	gfx.setTextColor(rgb(255,255,255));
	gfx.println("GRID");
	m.show();
}

void loop(SDLMatrix32 &m, uint32_t now)
{
    // do nothing for now
}

int main()
{
    // Desktop build should define GRID_EMULATOR_DESKTOP so SDLMatrix32 is available
    SDLMatrix32 m;
    m.begin();
    setup(m);
    uint32_t start = SDL_GetTicks();
    bool running = true;

    SDL_Event e;
    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                running = false;
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q)
                    running = false;
                else if (e.key.keysym.sym == SDLK_l) 
                    m.setLedMode(!m.ledMode());
            }
        }
        loop(m, millis_now(start));
        m.show();
    }
    return 0;
}
