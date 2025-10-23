#ifndef MAZE_SCENE_H
#define MAZE_SCENE_H

#include "Scene.h"
#include "Colors.h"
#include <climits>

struct Maze
{
    // Maze Config
    static constexpr uint8_t kMazeWidth = 15;
    static constexpr uint8_t kMazeHeight = 15;
    static constexpr uint8_t kMaxNeighbors = 4;
    static constexpr uint8_t kMaxEdges = (kMazeWidth * kMazeHeight) - 1;

    using coord = uint8_t;    // used for compressed position
    using matrix_t = uint8_t; // positions in Matrix-space
    using maze_t = uint8_t;   // positiongs in Maze-space

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
        coord pos = None;           // the position in the maze
        int weights[kMaxNeighbors]; // neighboring weights of edges of this node
        int value = INT_MAX;        // integer value to keep track of (cheapestEdgeWeight or distance)
        coord id = None;            // id of edge or node to keep track of
        bool used = false;          // whether the node has been used in the maze

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
        coord pos_relative(char dir)
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
        using vertex_list = std::vector<node *>;

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
};

class MazeScene : public Scene
{
private:
    // Color Config
    static const Color333 kPlayerColor;
    static const Color333 kWallColor;
    static const Color333 kStartColor;
    static const Color333 kFinishColor;
    static const Color333 kSolutionColor;
    static const Color333 kFoodColor;
    static const Color333 kTimeColor;
    // Start Scene config
    static const int8_t kStartLoopStopY = -60;   // vertical position to stop hints
    static const millis_t kStartLoopDelay = 150; // in ms
    static const char *textPlayer;
    static const char *textStart;
    static const char *textWall;
    static const char *textFinish;
    static const char *textHint;
    static const char *textFood;
    static const char *textTime;
    millis_t startTime = 0;
    int16_t textY;

    void renderHints(AppContext &ctx);

    // Maze State
    Color333 grid[MATRIX_HEIGHT][MATRIX_WIDTH]; // color of each pixel in matrix
    Maze::graph maze_g;                         // graph of maze
    Maze::node *startNode = nullptr;
    Maze::node *endNode = nullptr;

    // Generation

    void buildAdjacencyGraph(Maze::graph &adj_g);
    void setMazeEndpoints();
    void buildMaze();

    // Drawing

    void colorMaze();
    bool isBorder(Maze::matrix_t r, Maze::matrix_t col);
    void displayMaze(AppContext &ctx);

public:
    void setup(AppContext &ctx) override;
    void loop(AppContext &ctx) override;

    SceneKind kind() const override { return SceneKind::Maze; }
    const char *label() const override { return "Maze"; }
};

#endif // MAZE_SCENE_H
