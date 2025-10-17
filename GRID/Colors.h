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

// Color constants (3-3-3)
namespace Colors
{
    constexpr Color333 Black{0, 0, 0};
    namespace Muted
    {
        constexpr Color333 White{1, 1, 1};
        constexpr Color333 Red{1, 0, 0};
        constexpr Color333 Green{0, 1, 0};
        constexpr Color333 Blue{0, 0, 1};
        constexpr Color333 Yellow{1, 1, 0};
        constexpr Color333 Cyan{0, 1, 1};
        constexpr Color333 Violet{1, 0, 1}; // aka Magenta at full intensity
    }

    namespace Bright
    {
        constexpr Color333 Black{0, 0, 0};
        constexpr Color333 White{7, 7, 7};
        constexpr Color333 Red{7, 0, 0};
        constexpr Color333 Green{0, 7, 0};
        constexpr Color333 Blue{0, 0, 7};
        constexpr Color333 Yellow{7, 7, 0};
        constexpr Color333 Cyan{0, 7, 7};
        constexpr Color333 Violet{7, 0, 7}; // aka Magenta at full intensity
        constexpr Color333 Orange{7, 3, 0};
        constexpr Color333 Lime{4, 7, 0};
        constexpr Color333 Azure{0, 3, 7};
        constexpr Color333 Purple{4, 0, 7};
        constexpr Color333 Pink{7, 0, 4};
    }
}

#endif // COLORS_H
