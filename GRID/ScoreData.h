#ifndef SCORE_DATA_H
#define SCORE_DATA_H

// represents a Game score with name
struct ScoreData
{
    static constexpr uint8_t kMaxNameLength = 5;
    int score;                     // the integer score in the Maze game
    char name[kMaxNameLength + 1]; // +1 to leave space for NULL terminator
};

#endif // SCORE_DATA_H
