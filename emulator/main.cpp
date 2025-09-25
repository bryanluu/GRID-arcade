#include "GRID_App.h"
#include "SDL_Matrix32.h"
#include <SDL.h>
#include <cstdint>

static uint32_t millis_now(uint32_t start) { return SDL_GetTicks() - start; }

void setup(SDLMatrix32 &m)
{
    // sets alternating black and white pixels
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 32; x++)
        {
            RGB color = ((x + y) % 2 ? rgb(255, 255, 255) : rgb(0, 0, 0));
            m.set(x, y, color);
        }
    }
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
            }
        }
        m.show();
    }
    return 0;
}
