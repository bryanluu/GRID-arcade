#ifndef MAZE_SCENE_H
#define MAZE_SCENE_H

#include "Scene.h"
#include "Colors.h"
#include "ScrollTextHelper.h"
#include <climits>
#include <bitset>

struct Maze
{
    // Maze Config
    static constexpr uint8_t kMazeWidth = 15;
    static constexpr uint8_t kMazeHeight = 15;
    static constexpr uint8_t kMaxNeighbors = 4;
    static constexpr uint8_t kMaxEdges = (kMazeWidth * kMazeHeight) - 1;

    using coord = uint8_t;                   // used for compressed position
    using coords = std::vector<Maze::coord>; // list of compressed positions
    using matrix_t = uint8_t;                // positions in Matrix-space
    using maze_t = uint8_t;                  // positions in Maze-space
    using direction_t = uint8_t;             // relative directions of nodes

    enum Direction : int8_t
    {
        None = -1,
        Right,
        Down,
        Left,
        Up
    };

    class node
    {
    public:
        coord pos = None;               // the position in the maze
        int16_t weights[kMaxNeighbors]; // neighboring weights of edges of this node
        int16_t value = INT16_MAX;      // integer value to keep track of (cheapestEdgeWeight or distance)
        coord id = None;                // id of edge or node to keep track of
        bool used = false;              // whether the node has been used in the maze

        node()
        {
            for (coord i = 0; i < kMaxNeighbors; i++)
                weights[i] = None;
        }

        bool operator==(const node &other) const
        {
            return (pos == other.pos);
        }

        /**
         * @brief Find relative position from other
         *
         * @param other
         * @return Direction - a direction index
         */
        Direction operator-(const node &other) const
        {
            int8_t dx = getX(pos) - getX(other.pos);
            int8_t dy = getY(pos) - getY(other.pos);
            if (dx == 0)
            {
                if (dy == 1)
                    return Down;
                else if (dy == -1)
                    return Up;
            }
            else if (dy == 0)
            {
                if (dx == 1)
                    return Right;
                else if (dx == -1)
                    return Left;
            }
            return None;
        }

        /**
         * @brief Find the position by relative direction
         *
         * @param dir
         * @return coord - the position in the relative direction
         */
        coord pos_relative(direction_t dir)
        {
            int8_t dx, dy;
            dx = 0;
            dy = 0;
            switch (dir)
            {
            case Up:
                dy = -1;
                break;
            case Down:
                dy = 1;
                break;
            case Left:
                dx = -1;
                break;
            case Right:
                dx = 1;
                break;
            default:
                return None;
            }
            int8_t x, y;
            x = int8_t(getX(pos) + dx);
            y = int8_t(getY(pos) + dy);
            // if new position is invalid
            if (x < 0 || x >= kMazeWidth || y < 0 || y >= kMazeHeight)
                return None;

            return encode(x, y);
        }

        static bool compare(Maze::node *u, Maze::node *v);
    };

    // Encodes X,Y position into a compressed number
    static coord encode(maze_t x, maze_t y) { return x + (kMazeWidth * y); }

    // Returns the X column of the compressed position
    static maze_t getX(coord c) { return c % kMazeWidth; }

    // Returns the Y row of the compressed position
    static maze_t getY(coord c) { return c / kMazeWidth; }

    // Returns the Maze-space position of a Matrix-space position
    static maze_t toMaze(matrix_t p) { return (p - 1) / 2; }

    // Returns the Matrix-space position of a Maze-space position
    static matrix_t toMatrix(maze_t p) { return 2 * p + 1; }

    // Interpolate between maze coordinates in matrix space
    static matrix_t interpolate(maze_t p, maze_t q) { return p + q + 1; }

    class graph
    {
    public:
        static constexpr uint8_t size = kMazeWidth * kMazeHeight; // number of vertices
        node vertices[size];                                      // array of graph vertices

        graph() {}

        /**
         * Insert a new directed edge with a positive edge weight into the graph.
         *
         * @param s the encoded position of the source vertex for the edge
         * @param t the encoded position of the target vertex for the edge
         * @param weight the weight for the edge (has to be a positive integer)
         * @return true if the edge could be inserted or its weight updated, false if the edge with the
         *         same weight was already in the graph
         */
        bool insertEdge(coord s, coord t, int weight)
        {
            // source or target vertices invalid
            if (s >= size || t >= size)
                return false;

            // invalid negative weight
            if (weight < 0)
                return false;

            node &source = vertices[s];
            node &target = vertices[t];
            source.weights[(target) - (source)] = weight;
            target.weights[(source) - (target)] = weight;
            return true;
        }
    };

    static constexpr int16_t kMazePixels = 961; // 31 * 31
};

class MazeScene : public Scene
{
private:
    enum Stage : uint8_t
    {
        Intro,
        Game,
        End
    };
    Stage stage = Stage::Intro;
    void setStage(AppContext &ctx, Stage newStage);
    millis_t startTime = 0;

    // Color Config

    static const Color333 kPlayerColor;
    static const Colors::HSV::hue_t kWallHue;
    static const Colors::HSV::hue_t kStartHue;
    static const Colors::HSV::hue_t kFinishHue;
    static const Colors::HSV::hue_t kSolutionHue;
    static const Colors::HSV::hue_t kFoodHue;
    static const Colors::HSV::hue_t kTimeHue;

    // Start Scene config

    static const int8_t kStartTextYStop = -60; // vertical position to stop hints
    static const int8_t kStartTextY = 5;
    static const millis_t kStartTextStepRate = 150; // in ms
    static const char *textPlayer;
    static const char *textStart;
    static const char *textWall;
    static const char *textFinish;
    static const char *textHint;
    static const char *textFood;
    static const char *textTime;
    millis_t lastUpdateTime = 0;
    int8_t textY;

    void renderHints(AppContext &ctx, int8_t textY);

    // Maze State

    Maze::graph maze_g; // graph of maze
    Maze::node *startNode = nullptr;
    Maze::node *endNode = nullptr;
    Maze::matrix_t playerX, playerY;
    Maze::coords snacks;
    std::bitset<Maze::kMazePixels> seen; // saves memory: 961 bits ~120 bytes instead of 961 bytes

    // Generation

    void buildAdjacencyGraph(Maze::graph &adj_g);
    Maze::coord getFurthestIndex(Maze::coord src);
    void buildMaze();
    void setMazeEndpoints();
    void placeSnacks();

    // Movement

    Maze::Direction inputDir = Maze::Direction::None;
    static constexpr float kInputBuffer = 0.15f;        // deadband on normalized stick, ~15%
    static constexpr float kFastInputThreshold = 0.90f; // near edge speeds up repeat
    static constexpr millis_t kDefaultInputDelayMs = 150;
    static constexpr millis_t kFastInputDelayMs = 60;
    Maze::Direction sampleStrobedDirection(AppContext &ctx, Maze::Direction &ioDir, millis_t &ioLastTimeMs);
    void movePlayer(AppContext &ctx);

    // Visibility

    static constexpr Maze::matrix_t kVisibility = 1; // "Medium" visibility from old game

    // Snacks

    void eatSnack(Maze::coords::iterator pos, millis_t currentTime);
    void updateSnacks(millis_t currentTime);

    // Timer

    static constexpr uint8_t kTimerPixels = 63;                    // how many pixels used for timer
    static constexpr millis_t kGameDefaultDuration = (180 * 1000); // start with 3 mins to finish the game

    // Drawing

    static constexpr Colors::HSV::val_t kNearBrightness = 100; // 3/7 brightness
    static constexpr Colors::HSV::val_t kFarBrightness = 37;   // 1/7 on the brightness scale
    enum HuePalette : Colors::HSV::hue_t
    {
        None,
        Wall,
        Start,
        Finish,
        Player,
        Time,
        Food
    };
    HuePalette grid[MATRIX_HEIGHT][MATRIX_WIDTH]; // color of each pixel in matrix
    /**
     * @brief returns a color from the given hue and whether it should be the brighter variant
     */
    static Color333 palette(HuePalette hue, bool bright = false)
    {
        Colors::HSV::val_t val = (bright ? kNearBrightness : kFarBrightness);
        switch (hue)
        {
        case None:
            return Colors::Black;
        case Wall:
            return ColorHSV333(kWallHue, Colors::HSV::Saturation::Color, val);
        case Start:
            return ColorHSV333(kStartHue, Colors::HSV::Saturation::Color, val);
        case Finish:
            return ColorHSV333(kFinishHue, Colors::HSV::Saturation::Color, val);
        case Player:
            return kPlayerColor;
        case Time:
            return ColorHSV333(kTimeHue, Colors::HSV::Saturation::Color, val);
        case Food:
            return ColorHSV333(kFoodHue, Colors::HSV::Saturation::Color, val);
        default:
            return Colors::Black;
        }
    }

    void colorMaze();
    void colorStart();
    void colorFinish();
    void colorPlayer();
    void colorSnacks();
    bool isBorder(Maze::matrix_t r, Maze::matrix_t col);
    bool isNearPlayer(Maze::matrix_t x, Maze::matrix_t y);
    bool isOnMaze(Maze::matrix_t x, Maze::matrix_t y);
    void displayMaze(AppContext &ctx);
    void displayTimer(AppContext &ctx);
    bool playerHasFinished();
    void endGame(AppContext &ctx);

    // Scoring

    using score_t = uint8_t;
    score_t score;
    static constexpr score_t kExitScore = 20;    // score bonus if player reaches exit
    static constexpr score_t kMaxTimeScore = 50; // max score bonus from time left
    // in ms, the time after game start for player to achieve MAX_TIME_SCORE
    static constexpr millis_t kMaxTimeBuffer = (90 * 1000);
    static constexpr uint8_t kNumSnacks = 15;              // how many snacks to spawn
    static constexpr score_t kSnackPoints = 2;             // how many points does a snack give
    static constexpr millis_t kSnackTimeBoost = 1000;      // how much time does a snack give
    static constexpr millis_t kShowScoreDuration = (5000); // ms to show final score
    void computeFinalScore(score_t &score, millis_t nowMs);
    ScrollText banner;
    void showScore(AppContext &ctx, score_t score);

public:
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;

    SceneKind kind() const override { return SceneKind::Maze; }
    const char *label() const override { return "Maze"; }
};

#endif // MAZE_SCENE_H
