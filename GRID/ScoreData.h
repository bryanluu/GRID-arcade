#ifndef SCORE_DATA_H
#define SCORE_DATA_H

#include <cstdint>
#include <stddef.h>

// forward decl
struct IStorage;
struct ILogger;

// represents a Game score with name
struct ScoreData
{
    static constexpr uint8_t kMaxNameLength = 5;
    static constexpr size_t kJsonCap = 100;
    static constexpr size_t kReadCap = 100;
    static constexpr const char *kLogTag = "Score";
    static constexpr const char *kFileExtension = "dat";
    int score;                           // the integer score in the Maze game
    char name[kMaxNameLength + 1] = {0}; // +1 to leave space for NULL terminator

    // Serialize this object to JSON into dst.
    // Returns bytes written, or 0 on failure.
    size_t toJSON(char *dst, size_t cap) const;

    // Parse JSON into this object. Returns true on success.
    bool fromJSON(const char *src);

    // Save/load using IStorage (atomic write) with default filename.
    bool save(IStorage &storage, ILogger &log, const char *filename) const;
    bool load(IStorage &storage, ILogger &log, const char *filename);
};

#endif // SCORE_DATA_H
