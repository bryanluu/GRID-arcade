// IStorage.h
// Purpose: Small, portable interface for durable, whole-file persistence.
// Usage pattern:
// 1) storage.init("/save", appLogger);
// 2) storage.writeAll("calibration.json", bytes, n);   // atomic: temp -> flush -> rename
// 3) storage.readAll("calibration.json", buf, cap);    // read whole file into caller buffer
// 4) storage.exists/removeFile/removeTree as needed.
//
// Design notes:
// - No dynamic allocation in the interface; callers own buffers.
// - Backends may do boot-time temp-file recovery in init().
// - Errors carry both class (StorageError) and count (bytes).
#ifndef ISTORAGE_H
#define ISTORAGE_H

#include <stddef.h>
#include <stdint.h>

// Forward declare your logger interface to avoid pulling heavy headers here.
struct ILogger;
enum class LogLevel : uint8_t;

enum class StorageError
{
    None = 0,       // operation succeeded
    NotInitialized, // backend missing device/volume or required setup
    PathError,      // invalid or too-long path
    OpenFailed,     // failed to open/create file
    ReadFailed,     // short or failed read
    WriteFailed,    // short or failed write
    SyncFailed,     // failed to flush data/metadata to medium
    RenameFailed,   // failed to atomically replace final file
    RemoveFailed,   // failed to remove file/dir
    NotFound,       // file does not exist
    TooLarge,       // provided buffer too small for file contents
    MountFailed     // base directory missing and cannot be created
};

struct StorageResult
{
    StorageError err = StorageError::None;
    int32_t bytes = 0; // bytes read or written when applicable
    explicit operator bool() const { return err == StorageError::None; }
};

class IStorage
{
public:
    virtual ~IStorage() = default;

    // Ensure baseDir exists. May perform temp-file recovery (backend-specific).
    // The logger is non-owning and must outlive the storage object.
    virtual StorageResult init(const char *baseDir, ILogger *logger = nullptr) = 0;

    // Read the entire file into dst (cap bytes). Returns bytes read in .bytes.
    virtual StorageResult readAll(const char *relPath, void *dst, size_t cap) = 0;

    // Atomic write: write to a temp under baseDir, flush, then rename to relPath.
    // Returns bytes written in .bytes.
    virtual StorageResult writeAll(const char *relPath, const void *src, size_t nbytes) = 0;

    // Utilities: existence, single-file delete, and recursive delete.
    virtual bool exists(const char *relPath) = 0;
    virtual StorageResult removeFile(const char *relPath) = 0;
    virtual StorageResult removeTree(const char *relDir) = 0;

    // Base directory accessor (e.g., "/save").
    virtual const char *base() const = 0;
};

#endif // ISTORAGE_H
