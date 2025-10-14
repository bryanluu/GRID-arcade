// FileStorage.h — desktop filesystem backend
// Purpose: IStorage implementation using std::filesystem and std::fstream.
// Guarantees:
// - Atomic-ish update using temp file + rename (same directory).
// - No heap allocation beyond std::string internal usage.
// - Cleanup of a generic temp name on init (best effort).
#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#include "IStorage.h"
#include <string>

class FileStorage final : public IStorage
{
public:
    StorageResult init(const char *baseDir, StorageLogFn logger = nullptr) override;
    StorageResult readAll(const char *relPath, void *dst, size_t cap) override;
    StorageResult writeAll(const char *relPath, const void *src, size_t nbytes) override;
    bool exists(const char *relPath) override;
    StorageResult removeFile(const char *relPath) override;
    StorageResult removeTree(const char *relDir) override;
    const char *base() const override { return baseDir.c_str(); }

private:
    std::string baseDir = "/save";
    StorageLogFn log = nullptr;

    // Remove a leftover generic temp file (e.g., from abrupt termination).
    void recoverTemp();
};

#endif // FILESTORAGE_H
