// FlashStorage.cpp — Adafruit FatFs backend using ILogger (Arduino)
// Implementation notes:
// - makeAbs(): supports absolute relPath (starting with '/') and base-relative paths.
// - writeAll(): temp write -> f.sync() to push data and directory entry -> rename.
//   Minimizes corruption under power loss; worst case you lose only the “last write.”
// - readAll(): uses caller-provided buffer; avoids dynamic allocation on MCU.
// - removeTree(): uses rmRfStar() to recursively delete directory contents.
#include "FlashStorage.h"
#include "SdFat_Adafruit_Fork.h"
#include <Adafruit_SPIFlash.h>
#include <string.h>
#include "Logging.h" // ILogger, LogLevel

void FlashStorage::info(const char *msg)
{
    if (log)
        log->logf(LogLevel::Info, "[FlashStorage] %s", msg);
}
void FlashStorage::warn(const char *msg)
{
    if (log)
        log->logf(LogLevel::Warning, "[FlashStorage] %s", msg);
}

void FlashStorage::makeAbs(char *out, size_t cap, const char *base, const char *rel)
{
    if (rel && rel[0] == '/')
    { // already an absolute volume path
        strncpy(out, rel, cap);
        out[cap - 1] = 0;
        return;
    }
    snprintf(out, cap, "%s/%s", base, rel ? rel : "");
}

void FlashStorage::recoverTemp()
{
    // Generic cleanup: remove a known temp filename if present.
    // If you adopt temp names that include the target (e.g., "<name>.tmp"),
    // implement promotion to final here on boot.
    char tmp[256];
    makeAbs(tmp, sizeof(tmp), baseDir, ".tmp_write");
    if (vol.exists(tmp))
    {
        vol.remove(tmp);
        info("cleaned stale temp");
    }
}

StorageResult FlashStorage::init(const char *dir, ILogger *logger)
{
    log = logger;
    if (dir && *dir)
        baseDir = dir;

    // Ensure baseDir exists on the mounted volume.
    if (!vol.exists(baseDir) && !vol.mkdir(baseDir))
    {
        warn("base dir create failed");
        return {StorageError::MountFailed, 0};
    }

    // Cleanup any orphan temp from an interrupted write.
    recoverTemp();

    info("ready");
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

    // 1) Write to temp file; RAII ensures close even if we return early.
    {
        File32 f = vol.open(tmp, FILE_WRITE);
        if (!f)
        {
            warn("open temp failed");
            return {StorageError::OpenFailed, 0};
        }

        size_t w = f.write((const uint8_t *)src, n);
        f.sync(); // push data and FAT/dir updates to the medium

        if (w != n)
        {
            vol.remove(tmp); // cleanup partial temp
            warn("short write");
            return {StorageError::WriteFailed, (int32_t)w};
        }
        // f closes on scope exit
    }

    // 2) Replace final by rename (keep old file until rename succeeds).
    if (!vol.rename(tmp, abs))
    {
        vol.remove(tmp); // best-effort cleanup on failure
        warn("rename failed");
        return {StorageError::RenameFailed, 0};
    }

    info("write ok");
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
    {
        warn("buffer too small");
        return {StorageError::TooLarge, 0};
    }

    size_t r = f.read(dst, sz);
    if (r != sz)
    {
        warn("read failed");
        return {StorageError::ReadFailed, (int32_t)r};
    }

    return {StorageError::None, (int32_t)sz};
}

StorageResult FlashStorage::removeFile(const char *rel)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, rel);
    if (!vol.remove(abs))
    {
        warn("remove failed");
        return {StorageError::RemoveFailed, 0};
    }
    info("removed");
    return {};
}

StorageResult FlashStorage::removeTree(const char *relDir)
{
    char abs[256];
    makeAbs(abs, sizeof(abs), baseDir, relDir);

    // Open directory and recursively delete everything inside.
    File32 d = vol.open(abs);
    if (!d || !d.isDirectory())
        return {StorageError::PathError, 0};
    bool ok = d.rmRfStar();
    if (!ok)
    {
        warn("remove tree failed");
        return {StorageError::RemoveFailed, 0};
    }
    info("tree removed");
    return {};
}
