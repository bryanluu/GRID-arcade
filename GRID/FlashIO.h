// FlashIO.h — Adafruit FatFs backend (Arduino)
#ifndef FLASH_IO_H
#define FLASH_IO_H

#include "IExternalIO.h"

// Forward decls; provided and owned by your sketch.
class Adafruit_SPIFlash;
class FatVolume;
class File32;

class FlashIO final : public IExternalIO
{
public:
    FlashIO(Adafruit_SPIFlash &flash, FatVolume &vol) : flash(flash), vol(vol) {}

    IOResult init(const char *baseDir, LogFn logger = nullptr) override; // recovery included
    IOResult readAll(const char *relPath, void *dst, size_t cap) override;
    IOResult writeAll(const char *relPath, const void *src, size_t nbytes) override;
    bool exists(const char *relPath) override;
    IOResult removeFile(const char *relPath) override;
    IOResult removeTree(const char *relDir) override;
    const char *base() const override { return baseDir; }

private:
    const char *baseDir = "/save";
    LogFn log = nullptr;
    Adafruit_SPIFlash &flash; // non-owning
    FatVolume &vol;

    void recoverTemp(); // if final missing but temp exists, promote temp
    static void makeAbs(char *out, size_t cap, const char *base, const char *rel);
};

#endif // FLASH_IO_H
