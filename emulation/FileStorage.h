// FileStorage.h — desktop filesystem backend
#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#include "IStorage.h"
#include <string>

// Purpose: Implement IStorage using std::filesystem and std::fstream.
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

    // Remove generic leftover temp file (best-effort).
    void recoverTemp();
};

#endif // FILESTORAGE_H
