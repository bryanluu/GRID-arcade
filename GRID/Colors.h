#ifndef COLORS_H
#define COLORS_H

#include <cstdint>
#include <limits>

using Intensity3 = uint8_t;
using Intensity8 = uint8_t;

// Stores RGB pixel color
struct Color333
{
    Intensity3 r, g, b; // RGB are each 0..7

    constexpr bool operator==(const Color333 &other) const
    {
        return r == other.r && g == other.g && b == other.b;
    }

    constexpr bool operator!=(const Color333 &other) const
    {
        return !(*this == other);
    }
};
struct Color888
{
    Intensity8 r, g, b; // RGB are each 0..255
};

// Hue helper: degrees → internal hue units
// Internally we use 6 segments (R→Y→G→C→B→M) of 256 steps each = 1536 total.
constexpr uint16_t HUE_SEGMENTS = 6;                            // number of primary ramps
constexpr uint16_t HUE_SEGMENT_SIZE = 256;                      // steps per segment
constexpr uint16_t HUE_RANGE = HUE_SEGMENTS * HUE_SEGMENT_SIZE; // 1536

// Convert degrees (0..360) to Adafruit-compatible hue units (0..1535)
constexpr uint16_t HUE(double deg)
{
    // allow degrees outside 0..360; normalize is done by caller via modulo if needed.
    return static_cast<uint16_t>(HUE_RANGE * (deg / 360.0));
}

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

    namespace HSV
    {
        using hue_t = uint16_t;
        using sat_t = uint8_t;
        using val_t = uint8_t;

        namespace Hues
        {
            constexpr hue_t Red = HUE(0);
            constexpr hue_t Yellow = HUE(60);
            constexpr hue_t Green = HUE(120);
            constexpr hue_t Cyan = HUE(180);
            constexpr hue_t Blue = HUE(240);
            constexpr hue_t Violet = HUE(300);
        }

        namespace Saturation
        {
            constexpr sat_t Shade = 0;
            constexpr sat_t Color = UINT8_MAX;
        }
    }
}

// Channel and HSV ranges
constexpr uint8_t BYTE_MIN = 0;
constexpr uint8_t BYTE_MAX = 255; // 8-bit channel max
constexpr uint16_t HUE_MIN = 0;
constexpr uint16_t HUE_MAX = HUE_RANGE - 1; // 1535

// Optional gamma shaping
// This approximates gamma > 2.0 with two squaring passes.
// You can tune PASSES to 1 for ~gamma 2.0 feel, or keep 2 for stronger shaping.
constexpr bool GAMMA_DEFAULT_ENABLE = false;
constexpr uint8_t GAMMA_PASSES = 2; // 1 = mild, 2 = stronger

static inline uint8_t applyGamma(uint8_t c, bool enable)
{
    if (!enable || c == 0 || c == BYTE_MAX)
        return c;

    // Iteratively square and renormalize to 0..255 to approximate gamma
    uint16_t v = c;
    for (uint8_t i = 0; i < GAMMA_PASSES; ++i)
    {
        v = (v * v + (BYTE_MAX / 2)) / BYTE_MAX; // round to nearest
    }
    return static_cast<uint8_t>(v);
}

// helper to map 0..255 -> 0..7 with rounding as constexpr function (avoid lambda)
constexpr uint8_t to3bit(uint8_t x)
{
    return static_cast<uint8_t>((static_cast<uint16_t>(x) * 7 + (BYTE_MAX / 2)) / BYTE_MAX);
}

// HSV( hue: 0..1535, sat: 0..255, val: 0..255 ) → 8-bit RGB
static inline Color888 ColorHSV888(uint16_t hue, uint8_t sat, uint8_t val, bool gamma = GAMMA_DEFAULT_ENABLE)
{
    // Normalize hue into [0, HUE_MAX]
    hue = static_cast<uint16_t>(hue % HUE_RANGE);

    // Fully desaturated → grayscale
    if (sat == 0)
    {
        uint8_t g = applyGamma(val, gamma);
        return Color888{g, g, g};
    }

    // Identify which primary segment we are in and our position within it
    const uint8_t region = static_cast<uint8_t>(hue / HUE_SEGMENT_SIZE); // 0..5
    const uint8_t pos = static_cast<uint8_t>(hue % HUE_SEGMENT_SIZE);    // 0..255

    // Precompute the three auxiliaries used by HSV→RGB
    // p = v*(1-s)
    // q = v*(1-s*f)
    // t = v*(1-s*(1-f))
    const uint16_t p = static_cast<uint16_t>(val) * (BYTE_MAX - sat) / BYTE_MAX;
    const uint16_t q = static_cast<uint16_t>(val) *
                       (BYTE_MAX - static_cast<uint16_t>(sat) * pos / BYTE_MAX) / BYTE_MAX;
    const uint16_t t = static_cast<uint16_t>(val) *
                       (BYTE_MAX - static_cast<uint16_t>(sat) * (HUE_SEGMENT_SIZE - 1 - pos) / BYTE_MAX) / BYTE_MAX;

    // Initialize to safe defaults so constexpr path-checking is happy
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    switch (region)
    {
    default: // 0: Red → Yellow
    case 0:
        r = val;
        g = static_cast<uint8_t>(t);
        b = static_cast<uint8_t>(p);
        break;
    case 1:
        r = static_cast<uint8_t>(q);
        g = val;
        b = static_cast<uint8_t>(p);
        break; // 1: Yellow → Green
    case 2:
        r = static_cast<uint8_t>(p);
        g = val;
        b = static_cast<uint8_t>(t);
        break; // 2: Green → Cyan
    case 3:
        r = static_cast<uint8_t>(p);
        g = static_cast<uint8_t>(q);
        b = val;
        break; // 3: Cyan → Blue
    case 4:
        r = static_cast<uint8_t>(t);
        g = static_cast<uint8_t>(p);
        b = val;
        break; // 4: Blue → Magenta
    case 5:
        r = val;
        g = static_cast<uint8_t>(p);
        b = static_cast<uint8_t>(q);
        break; // 5: Magenta → Red
    }

    // Optional gamma shaping
    if (gamma)
    {
        r = applyGamma(r, true);
        g = applyGamma(g, true);
        b = applyGamma(b, true);
    }
    return Color888{r, g, b};
}

// HSV → 3-3-3 RGB (down-quantized for Color333)
static inline Color333 ColorHSV333(uint16_t hue, uint8_t sat, uint8_t val, bool gamma = GAMMA_DEFAULT_ENABLE)
{
    const Color888 c8 = ColorHSV888(hue, sat, val, gamma);
    return Color333{to3bit(c8.r), to3bit(c8.g), to3bit(c8.b)};
}

#endif // COLORS_H
