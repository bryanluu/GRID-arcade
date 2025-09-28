#ifndef COLORS_H
#define COLORS_H

#include <cstdint>

// Stores RGB pixel color as 0..7 each channel
struct Color333
{
    uint8_t r, g, b;
};

#define BLACK   (Color333{0,0,0})
#define WHITE   (Color333{7,7,7})
#define RED     (Color333{7,0,0})
#define GREEN   (Color333{0,7,0})
#define BLUE    (Color333{0,0,7})
#define YELLOW  (Color333{7,7,0})
#define CYAN    (Color333{0,7,7})
#define VIOLET  (Color333{7,0,7})
#define ORANGE  (Color333{7,3,0})
#define LIME    (Color333{4,7,0})
#define AZURE   (Color333{0,3,7})
#define PURPLE  (Color333{4,0,7})
#define PINK    (Color333{7,0,4})

#endif // COLORS_H
