// IStorage.h
#ifndef ISTORAGE_H
#define ISTORAGE_H

#include <stddef.h>
#include <stdint.h>

// Purpose: Small, portable interface for whole-file persistence.
enum class StorageError
{
    None = 0,
    NotInitialized,
    PathError,
    OpenFailed,
    ReadFailed,
    WriteFailed,
    SyncFailed,
    RenameFailed,
    RemoveFailed,
    NotFound,
    TooLarge,
    MountFailed
};

struct StorageResult
{
    StorageError err = StorageError::None;
    int32_t bytes = 0; // bytes read/written when applicable
    explicit operator bool() const { return err == StorageError::None; }
};

// Optional lightweight logger (e.g., Serial.println).
using StorageLogFn = void (*)(const char *);

class IStorage
{
public:
    virtual ~IStorage() = default;

    // Initialize backend and ensure baseDir exists. May perform recovery.
    virtual StorageResult init(const char *baseDir, StorageLogFn logger = nullptr) = 0;

    // Whole-file operations relative to baseDir.
    virtual StorageResult readAll(const char *relPath, void *dst, size_t cap) = 0;
    virtual StorageResult writeAll(const char *relPath, const void *src, size_t nbytes) = 0;

    virtual bool exists(const char *relPath) = 0;
    virtual StorageResult removeFile(const char *relPath) = 0;
    virtual StorageResult removeTree(const char *relDir) = 0;

    virtual const char *base() const = 0;
};

#endif // ISTORAGE_H
