// Serializer.h
// Purpose:
// - Provide compact, dependency-free JSON serialization helpers for small data structs.
// - Starts with InputCalibration; add more nested structs (e.g., Score) over time.
//
// Design:
// - No dynamic allocation inside helpers; callers provide buffers.
// - Short JSON keys to minimize bytes on MCU storage.
// - Tolerant parser: missing fields keep defaults in 'out'.
//
// Usage:
//   char json[192];
//   size_t n = Serializer::Calibration::toJSON(cal, json, sizeof(json));
//   InputCalibration out{};
//   bool ok = Serializer::Calibration::fromJSON(json, out);
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stddef.h>
#include <stdint.h>

struct InputCalibration; // forward decl

struct Serializer
{
    // Serialization helpers for InputCalibration.
    // JSON shape:
    //   {"v":1,"dz":0.08,"gm":1.8,"xl":0,"xc":512,"xh":1023,"yl":0,"yc":512,"yh":1023}
    struct Calibration
    {
        // Serialize 'c' to JSON into 'dst' with capacity 'cap'.
        // Returns: number of bytes written (excluding terminating '\0'), or 0 on failure.
        // Notes:
        // - Uses short keys to save bytes: v,dz,gm,xl,xc,xh,yl,yc,yh.
        // - Float formatting uses %.4g for compactness and readability.
        static size_t toJSON(const InputCalibration &c, char *dst, size_t cap);

        // Parse JSON from 'src' (null-terminated) into 'out'.
        // Returns: true on success, false on invalid input.
        // Notes:
        // - Missing fields are left at 'out' defaults.
        // - Extra fields are ignored.
        // - Version ("v") is optional and reserved for future migrations.
        static bool fromJSON(const char *src, InputCalibration &out);
    };
};

#endif // SERIALIZER_H
