// FlashStorage.h — Adafruit FatFs backend using ILogger (Arduino)
// Purpose: IStorage implementation on Adafruit SPI Flash + FatFs.
//
// Lifecycle:
// - In setup(), initialize flash and mount the FatFs volume.
// - Construct FlashStorage(flash, vol) with non-owning references.
// - Call init("/save", logger) to ensure base dir exists and cleanup stale temp.
//
// Safety under power loss:
// - writeAll() uses temp -> f.sync() -> rename, keeping the old file until rename succeeds.
// - init() removes a generic orphan temp left by interrupted writes.
// - For “promote temp to final,” encode the target in temp name and extend recoverTemp().
#ifndef FLASHSTORAGE_H
#define FLASHSTORAGE_H

#include "IStorage.h"

// Forward declarations to keep the interface light; the sketch provides these.
class Adafruit_SPIFlash;
class FatVolume;
class File32;

class FlashStorage final : public IStorage
{
public:
    // Non-owning references; caller controls lifetime and mounting.
    FlashStorage(Adafruit_SPIFlash &flash, FatVolume &vol) : flash(flash), vol(vol) {}

    StorageResult init(const char *baseDir, ILogger *logger = nullptr) override; // includes temp cleanup
    StorageResult readAll(const char *relPath, void *dst, size_t cap) override;
    StorageResult writeAll(const char *relPath, const void *src, size_t nbytes) override;
    bool exists(const char *relPath) override;
    StorageResult removeFile(const char *relPath) override;
    StorageResult removeTree(const char *relDir) override;
    const char *base() const override { return baseDir; }

private:
    const char *baseDir = "/save"; // Volume-relative root for game saves
    ILogger *log = nullptr;        // non-owning
    Adafruit_SPIFlash &flash;      // non-owning
    FatVolume &vol;                // non-owning

    // Remove a generic orphan temp file. Extend to promote temp if needed.
    void recoverTemp();

    // Build absolute volume path from base + relative; respects already-absolute rel.
    static void makeAbs(char *out, size_t cap, const char *base, const char *rel);

    // Logging helpers
    void info(const char *msg);
    void warn(const char *msg);
};

#endif // FLASHSTORAGE_H
