#ifndef COLORS_H
#define COLORS_H

#include <cstdint>

using Intensity3 = uint8_t;
using Intensity8 = uint8_t;

// Stores RGB pixel color
struct Color333
{
    Intensity3 r, g, b; // RGB are each 0..7
};
struct Color888
{
    Intensity8 r, g, b; // RGB are each 0..255
};

#define BLACK (Color333{0, 0, 0})
#define WHITE (Color333{7, 7, 7})
#define RED (Color333{7, 0, 0})
#define GREEN (Color333{0, 7, 0})
#define BLUE (Color333{0, 0, 7})
#define YELLOW (Color333{7, 7, 0})
#define CYAN (Color333{0, 7, 7})
#define VIOLET (Color333{7, 0, 7})
#define ORANGE (Color333{7, 3, 0})
#define LIME (Color333{4, 7, 0})
#define AZURE (Color333{0, 3, 7})
#define PURPLE (Color333{4, 0, 7})
#define PINK (Color333{7, 0, 4})
#define GRAY (Color333{1, 1, 1})

#endif // COLORS_H
