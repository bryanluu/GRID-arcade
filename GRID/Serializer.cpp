// Serializer.cpp
// Rationale:
// - Hand-rolled JSON keeps code size small and avoids external deps.
// - The parser is key-oriented and tolerant: it finds numbers by key labels.
// - Suitable for tiny payloads (<< 512 bytes).
#include "Serializer.h"
#include "Input.h"
#include <stdio.h>  // snprintf
#include <stdlib.h> // strtol, strtof
#include <string.h> // strstr, strchr

// Internal helper: find integer at key "key" in 's' and store into 'v'.
// Returns true on success; does not modify 'v' if key is not present.
static bool findNumI(const char *s, const char *key, long &v)
{
    const char *p = strstr(s, key);
    if (!p)
        return false;
    p = strchr(p, ':');
    if (!p)
        return false;
    char *end = nullptr;
    long val = strtol(p + 1, &end, 10);
    if (end == p + 1)
        return false; // no digits parsed
    v = val;
    return true;
}

// Internal helper: find float at key "key" in 's' and store into 'v'.
// Returns true on success; does not modify 'v' if key is not present.
static bool findNumF(const char *s, const char *key, float &v)
{
    const char *p = strstr(s, key);
    if (!p)
        return false;
    p = strchr(p, ':');
    if (!p)
        return false;
    char *end = nullptr;
    float val = strtof(p + 1, &end);
    if (end == p + 1)
        return false; // no digits parsed
    v = val;
    return true;
}

size_t Serializer::Calibration::toJSON(const InputCalibration &c, char *dst, size_t cap)
{
    // Emit a compact, order-stable JSON object.
    // Order is helpful for diffs and reproducibility.
    int n = snprintf(dst, cap,
                     "{\"v\":%u,\"dz\":%.4g,\"gm\":%.4g,"
                     "\"xl\":%u,\"xc\":%u,\"xh\":%u,"
                     "\"yl\":%u,\"yc\":%u,\"yh\":%u}",
                     1u,
                     c.deadzone, c.gamma,
                     (unsigned)c.x_adc_low, (unsigned)c.x_adc_center, (unsigned)c.x_adc_high,
                     (unsigned)c.y_adc_low, (unsigned)c.y_adc_center, (unsigned)c.y_adc_high);
    // snprintf returns number of chars that would have been written (excluding '\0')
    // if >= cap, output was truncated -> treat as failure (return 0).
    if (n <= 0 || (size_t)n >= cap)
        return 0;
    return (size_t)n;
}

bool Serializer::Calibration::fromJSON(const char *src, InputCalibration &out)
{
    // Basic validation
    if (!src || !*src)
        return false;

    // Version is optional and currently unused; read and ignore.
    long vi = 0;
    (void)vi;
    findNumI(src, "\"v\"", vi);

    // Floats
    float f = 0.f;
    if (findNumF(src, "\"dz\"", f))
        out.deadzone = f;
    if (findNumF(src, "\"gm\"", f))
        out.gamma = f;

    // Integers (ADC points)
    long li = 0;
    if (findNumI(src, "\"xl\"", li))
        out.x_adc_low = (uint16_t)li;
    if (findNumI(src, "\"xc\"", li))
        out.x_adc_center = (uint16_t)li;
    if (findNumI(src, "\"xh\"", li))
        out.x_adc_high = (uint16_t)li;
    if (findNumI(src, "\"yl\"", li))
        out.y_adc_low = (uint16_t)li;
    if (findNumI(src, "\"yc\"", li))
        out.y_adc_center = (uint16_t)li;
    if (findNumI(src, "\"yh\"", li))
        out.y_adc_high = (uint16_t)li;

    // If the JSON was present but missing some keys, we still succeed and
    // keep the default values in 'out' for the missing keys.
    return true;
}
