// FlashIO.cpp — Adafruit FatFs backend (Arduino)
#include "FlashIO.h"
#include "SdFat_Adafruit_Fork.h"
#include <Adafruit_SPIFlash.h>
#include <string.h>

void FlashIO::makeAbs(char *out, size_t cap, const char *base, const char *rel)
{
    if (rel && rel[0] == '/')
    {
        strncpy(out, rel, cap);
        out[cap - 1] = 0;
        return;
    }
    snprintf(out, cap, "%s/%s", base, rel ? rel : "");
}

void FlashIO::recoverTemp()
{
    // Promote leftover temp if final missing. We only know the shared temp name.
    char tmp[256];
    makeAbs(tmp, sizeof(tmp), baseDir, ".tmp_write");
    if (!vol.exists(tmp))
        return;
    // If you have specific target file names, you can map temp->target; here we just remove temp.
    vol.remove(tmp);
}

IOResult FlashIO::init(const char *dir, LogFn logger)
{
    log = logger;
    if (dir && *dir)
        baseDir = dir;
    if (!vol.exists(baseDir) && !vol.mkdir(baseDir))
        return {IOError::MountFailed, 0};
    recoverTemp();
    if (log)
        log("[FlashIO] ready");
    return {};
}

bool FlashIO::exists(const char *rel)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, rel);
    return vol.exists(abs);
}

IOResult FlashIO::writeAll(const char *rel, const void *src, size_t n)
{
    char abs[256], tmp[256];
    makeAbs(abs, sizeof(abs), baseDir, rel);
    makeAbs(tmp, sizeof(tmp), baseDir, ".tmp_write");

    {
        File32 f = vol.open(tmp, FILE_WRITE); // RAII close on scope exit
        if (!f)
            return {IOError::OpenFailed, 0};
        size_t w = f.write((const uint8_t *)src, n);
        f.sync(); // push data and directory updates before rename
        if (w != n)
        {
            vol.remove(tmp);
            return {IOError::WriteFailed, (int32_t)w};
        }
    }

    // Do NOT remove old file first; keep last good copy if power cuts.
    if (!vol.rename(tmp, abs))
    {
        vol.remove(tmp);
        return {IOError::RenameFailed, 0};
    }

    // Optional: reopen final and sync to reduce metadata window (often not necessary).
    // File32 g = vol.open(abs, FILE_READ); if (g) { g.sync(); } // implicit close

    return {IOError::None, (int32_t)n};
}

IOResult FlashIO::readAll(const char *rel, void *dst, size_t cap)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, rel);
    File32 f = vol.open(abs, FILE_READ); // RAII
    if (!f)
        return {IOError::NotFound, 0};
    size_t sz = (size_t)f.size();
    if (sz >= cap)
        return {IOError::TooLarge, 0};
    size_t r = f.read(dst, sz);
    if (r != sz)
        return {IOError::ReadFailed, (int32_t)r};
    return {IOError::None, (int32_t)sz};
}

IOResult FlashIO::removeFile(const char *rel)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, rel);
    return vol.remove(abs) ? IOResult{} : IOResult{IOError::RemoveFailed, 0};
}

IOResult FlashIO::removeTree(const char *relDir)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, relDir);
    File32 d = vol.open(abs);
    if (!d || !d.isDirectory())
        return {IOError::PathError, 0};
    bool ok = d.rmRfStar();
    return ok ? IOResult{} : IOResult{IOError::RemoveFailed, 0};
}
