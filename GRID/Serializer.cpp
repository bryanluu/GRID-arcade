// Serializer.cpp
// Rationale:
// - Hand-rolled JSON keeps code size small and avoids external deps.
// - The parser is key-oriented and tolerant: it finds numbers by key labels.
// - Suitable for tiny payloads (<< 512 bytes).
#include "Serializer.h"
#include "Input.h"
#include "ScoreData.h"
#include <stdio.h>  // snprintf
#include <stdlib.h> // strtol, strtof
#include <string.h> // strstr, strchr
#include <math.h>

// Formats v into out as a compact decimal with up to maxDec digits after dot.
// Examples: 0 -> "0", -0.08 -> "-0.08", 1.8 -> "1.8", 1.23456 -> "1.2346"
// We need this on Arduino because it does not support %g formats for floats.
static bool formatFloatCompact(float v, char *out, size_t cap, uint8_t maxDec = 4)
{
    if (!out || cap < 2)
        return false;

    // Handle NaN/Inf gracefully
    if (isnan(v) || isinf(v))
    {
        // Choose your policy; here we write "0"
        if (cap < 2)
            return false;
        out[0] = '0';
        out[1] = '\0';
        return true;
    }

    // Work with absolute value, remember sign
    bool neg = v < 0.0f;
    float av = neg ? -v : v;

    // Integer and scaled fractional parts
    uint32_t intPart = (uint32_t)av;
    float fracF = av - (float)intPart;

    // Scale fractional to integer with rounding at maxDec
    uint32_t scale = 1;
    for (uint8_t i = 0; i < maxDec; ++i)
        scale *= 10;
    uint32_t frac = (uint32_t)lroundf(fracF * (float)scale);

    // Handle carry from rounding (e.g., 1.99995 -> 2.0000)
    if (frac >= scale)
    {
        frac = 0;
        intPart += 1;
    }

    // Trim trailing zeros in fractional
    uint8_t decs = maxDec;
    while (decs > 0 && (frac % 10u) == 0u)
    {
        frac /= 10u;
        --decs;
    }

    // Build into out
    int n = 0;
    if (neg)
    {
        if (cap < 2)
            return false;
        out[n++] = '-';
    }
    // write integer
    // use a small temp to print intPart
    char ibuf[16];
    int in = snprintf(ibuf, sizeof(ibuf), "%u", (unsigned)intPart);
    if (in <= 0)
        return false;
    if ((size_t)(n + in) >= cap)
        return false;
    memcpy(out + n, ibuf, (size_t)in);
    n += in;

    // fractional (optional)
    if (decs > 0)
    {
        if ((size_t)(n + 1) >= cap)
            return false;
        out[n++] = '.';
        // zero-pad fractional to 'decs' digits
        // write into fbuf then left-pad with zeros if needed
        char fbuf[16];
        int fn = snprintf(fbuf, sizeof(fbuf), "%u", (unsigned)frac);
        if (fn <= 0)
            return false;
        int pad = (int)decs - fn;
        if ((size_t)(n + decs) >= cap)
            return false;
        while (pad-- > 0)
            out[n++] = '0';
        memcpy(out + n, fbuf, (size_t)fn);
        n += fn;
    }

    if ((size_t)n >= cap)
        return false;
    out[n] = '\0';
    return true;
}

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

// Internal helper: find str at key "key" in 's' and store into 'buf'.
// Returns true on success; does not modify 'v' if key is not present.
static bool findStr(const char *s, const char *key, char *buf)
{
    const char *p = strstr(s, key);
    if (!p)
        return false;
    p = strchr(p, ':');
    if (!p)
        return false;
    p = strchr(p, '"');
    if (!p)
        return false;
    const char *end = strrchr(p + 1, '"');
    if (end == p + 1)
        return false; // missing closing quote
    memcpy(buf, p + 1, ScoreData::kMaxNameLength);
    return true;
}

size_t Serializer::Calibration::toJSON(const InputCalibration &c, char *dst, size_t cap)
{
    // Prepare float fields as strings since many Arduino libc builds lack printf float.
    char dz[24], gm[24];
    if (!formatFloatCompact(c.deadzone, dz, sizeof(dz), 4))
        return 0;
    if (!formatFloatCompact(c.gamma, gm, sizeof(gm), 4))
        return 0;

    // Emit compact, order-stable JSON. Inject float strings via %s.
    int n = snprintf(dst, cap,
                     "{\"v\":%u,\"dz\":%s,\"gm\":%s,"
                     "\"xl\":%u,\"xc\":%u,\"xh\":%u,"
                     "\"yl\":%u,\"yc\":%u,\"yh\":%u}",
                     1u,
                     dz, gm,
                     (unsigned)c.x_adc_low, (unsigned)c.x_adc_center, (unsigned)c.x_adc_high,
                     (unsigned)c.y_adc_low, (unsigned)c.y_adc_center, (unsigned)c.y_adc_high);
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

bool Serializer::Score::fromJSON(const char *src, ScoreData &out)
{
    // Basic validation
    if (!src || !*src)
        return false;

    // Version is optional and currently unused; read and ignore.
    long vi = 0;
    (void)vi;
    findNumI(src, "\"v\"", vi);

    char buf[ScoreData::kMaxNameLength];
    if (findStr(src, "\"n\"", buf))
        memcpy(out.name, buf, ScoreData::kMaxNameLength);

    // Integers (ADC points)
    long li = 0;
    if (findNumI(src, "\"s\"", li))
        out.score = (uint16_t)li;

    // If the JSON was present but missing some keys, we still succeed and
    // keep the default values in 'out' for the missing keys.
    return true;
}
