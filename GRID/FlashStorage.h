// FlashStorage.h — Adafruit FatFs backend using ILogger (Arduino)
// Purpose: Implement IStorage on Adafruit SPI Flash + FatFs.
// Lifecycle (sketch):
//   - Initialize flash and mount volume in setup()
//   - FlashStorage storage(flash, vol);
//   - storage.init("/save", appLogger);
// Safety under power loss:
//   - writeAll(): temp write -> f.sync() -> rename; old file kept until rename succeeds.
//   - init(): cleans up a generic orphan temp file from interrupted writes.
//   - For temp promotion, encode target name in temp file and extend recoverTemp().
#ifndef FLASHSTORAGE_H
#define FLASHSTORAGE_H

#include "IStorage.h"

// Forward declarations — provided by your sketch / Adafruit libs.
class Adafruit_SPIFlash;
class FatVolume;
class File32;

class FlashStorage final : public IStorage
{
public:
    // Defaults used if caller passes nullptr or empty string to init()
    static constexpr const char *kDefaultBaseDir = "/save";
    static constexpr const char *kTempName = ".tmp_write";
    static constexpr size_t kPathCap = 256; // on-stack path buffers

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
    const char *baseDir = kDefaultBaseDir; // Volume-relative root for saves
    ILogger *log = nullptr;                // non-owning
    Adafruit_SPIFlash &flash;              // non-owning
    FatVolume &vol;                        // non-owning

    // Remove a generic orphan temp. Extend to promote temp if you encode target in the temp name.
    void recoverTemp();

    // Build absolute volume path from base + relative; respects already-absolute rel.
    static void makeAbs(char *out, size_t cap, const char *base, const char *rel);

    // Logging helpers — include “[FlashStorage]” prefix here to keep call sites clean.
    void info(const char *msg);
    void warn(const char *msg);
};

#endif // FLASHSTORAGE_H
