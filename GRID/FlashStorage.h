// FlashStorage.h — Adafruit FatFs backend (Arduino)
#ifndef FLASHSTORAGE_H
#define FLASHSTORAGE_H

#include "IStorage.h"

// Forward declarations (owned by sketch).
class Adafruit_SPIFlash;
class FatVolume;
class File32;

// Purpose: Implement IStorage on Adafruit SPI Flash + FatFs.
class FlashStorage final : public IStorage
{
public:
    FlashStorage(Adafruit_SPIFlash &flash, FatVolume &vol) : flash(flash), vol(vol) {}

    StorageResult init(const char *baseDir, StorageLogFn logger = nullptr) override; // includes temp cleanup
    StorageResult readAll(const char *relPath, void *dst, size_t cap) override;
    StorageResult writeAll(const char *relPath, const void *src, size_t nbytes) override;
    bool exists(const char *relPath) override;
    StorageResult removeFile(const char *relPath) override;
    StorageResult removeTree(const char *relDir) override;
    const char *base() const override { return baseDir; }

private:
    const char *baseDir = "/save";
    StorageLogFn log = nullptr;
    Adafruit_SPIFlash &flash; // non-owning
    FatVolume &vol;

    void recoverTemp(); // remove stale temp from interrupted writes
    static void makeAbs(char *out, size_t cap, const char *base, const char *rel);
};

#endif // FLASHSTORAGE_H
