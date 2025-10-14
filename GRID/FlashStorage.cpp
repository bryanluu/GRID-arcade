// FlashStorage.cpp — Adafruit FatFs backend (Arduino)
#include "FlashStorage.h"
#include "SdFat_Adafruit_Fork.h"
#include <Adafruit_SPIFlash.h>
#include <string.h>

void FlashStorage::makeAbs(char *out, size_t cap, const char *base, const char *rel)
{
    if (rel && rel[0] == '/')
    {
        strncpy(out, rel, cap);
        out[cap - 1] = 0;
        return;
    }
    snprintf(out, cap, "%s/%s", base, rel ? rel : "");
}

void FlashStorage::recoverTemp()
{
    // Generic cleanup: remove known temp if present.
    char tmp[256];
    makeAbs(tmp, sizeof(tmp), baseDir, ".tmp_write");
    if (vol.exists(tmp))
    {
        vol.remove(tmp);
    }
}

StorageResult FlashStorage::init(const char *dir, StorageLogFn logger)
{
    log = logger;
    if (dir && *dir)
        baseDir = dir;
    if (!vol.exists(baseDir) && !vol.mkdir(baseDir))
        return {StorageError::MountFailed, 0};
    recoverTemp();
    if (log)
        log("[FlashStorage] ready");
    return {};
}

bool FlashStorage::exists(const char *rel)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, rel);
    return vol.exists(abs);
}

StorageResult FlashStorage::writeAll(const char *rel, const void *src, size_t n)
{
    char abs[256], tmp[256];
    makeAbs(abs, sizeof(abs), baseDir, rel);
    makeAbs(tmp, sizeof(tmp), baseDir, ".tmp_write");

    {
        File32 f = vol.open(tmp, FILE_WRITE); // RAII close
        if (!f)
            return {StorageError::OpenFailed, 0};
        size_t w = f.write((const uint8_t *)src, n);
        f.sync(); // push data + metadata before rename
        if (w != n)
        {
            vol.remove(tmp);
            return {StorageError::WriteFailed, (int32_t)w};
        }
    }
    // Keep old file; only replace on successful rename.
    if (!vol.rename(tmp, abs))
    {
        vol.remove(tmp);
        return {StorageError::RenameFailed, 0};
    }
    return {StorageError::None, (int32_t)n};
}

StorageResult FlashStorage::readAll(const char *rel, void *dst, size_t cap)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, rel);
    File32 f = vol.open(abs, FILE_READ); // RAII
    if (!f)
        return {StorageError::NotFound, 0};
    size_t sz = (size_t)f.size();
    if (sz >= cap)
        return {StorageError::TooLarge, 0};
    size_t r = f.read(dst, sz);
    if (r != sz)
        return {StorageError::ReadFailed, (int32_t)r};
    return {StorageError::None, (int32_t)sz};
}

StorageResult FlashStorage::removeFile(const char *rel)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, rel);
    return vol.remove(abs) ? StorageResult{} : StorageResult{StorageError::RemoveFailed, 0};
}

StorageResult FlashStorage::removeTree(const char *relDir)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, relDir);
    File32 d = vol.open(abs);
    if (!d || !d.isDirectory())
        return {StorageError::PathError, 0};
    bool ok = d.rmRfStar();
    return ok ? StorageResult{} : StorageResult{StorageError::RemoveFailed, 0};
}
