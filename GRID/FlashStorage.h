// FlashStorage.h — Adafruit FatFs backend (Arduino)
// Purpose: IStorage implementation on Adafruit SPI Flash + FatFs.
// Lifecycle: in setup(), initialize flash and mount the volume, then:
//   FlashStorage storage(flash, vol);
//   storage.init("/save", Serial.println);
// Safety:
// - writeAll() uses temp -> f.sync() -> rename, keeping old file until rename succeeds.
// - init() cleans up a generic orphan temp left by interrupted writes.
// - For “promote temp to final” recovery, encode the target name in the temp and extend recover logic.
#ifndef FLASHSTORAGE_H
#define FLASHSTORAGE_H

#include "IStorage.h"

// Forward declarations to keep the interface light; these are provided by your sketch.
class Adafruit_SPIFlash;
class FatVolume;
class File32;

class FlashStorage final : public IStorage
{
public:
    // Non-owning references; caller controls lifetime and mounting.
    FlashStorage(Adafruit_SPIFlash &flash, FatVolume &vol) : flash(flash), vol(vol) {}

    StorageResult init(const char *baseDir, StorageLogFn logger = nullptr) override; // includes temp cleanup
    StorageResult readAll(const char *relPath, void *dst, size_t cap) override;
    StorageResult writeAll(const char *relPath, const void *src, size_t nbytes) override;
    bool exists(const char *relPath) override;
    StorageResult removeFile(const char *relPath) override;
    StorageResult removeTree(const char *relDir) override;
    const char *base() const override { return baseDir; }

private:
    const char *baseDir = "/save"; // Volume-relative root for game saves
    StorageLogFn log = nullptr;
    Adafruit_SPIFlash &flash; // Provided and owned by the sketch
    FatVolume &vol;           // Provided and owned by the sketch

    // Remove a generic orphan temp file. Extend to promote temp if needed.
    void recoverTemp();

    // Build absolute volume path from base + relative; respects already-absolute rel.
    static void makeAbs(char *out, size_t cap, const char *base, const char *rel);
};

#endif // FLASHSTORAGE_H
