#include "MazeScene.h"
#include "Helpers.h"
#include "SceneBus.h"
#include <vector>
#include <queue>

const char *MazeScene::textPlayer = "You";
const char *MazeScene::textStart = "Start";
const char *MazeScene::textWall = "Wall";
const char *MazeScene::textFinish = "Exit";
const char *MazeScene::textHint = "Hint";
const char *MazeScene::textFood = "Food";
const char *MazeScene::textTime = "Time";

const Color333 MazeScene::kPlayerColor = Colors::Muted::White;
const Color333 MazeScene::kWallColor = Colors::Muted::Red;
const Color333 MazeScene::kStartColor = Colors::Muted::Blue;
const Color333 MazeScene::kFinishColor = Colors::Muted::Green;
const Color333 MazeScene::kSolutionColor = Colors::Muted::Yellow;
const Color333 MazeScene::kFoodColor = Colors::Muted::Cyan;
const Color333 MazeScene::kTimeColor = Colors::Muted::Violet;

// ########## START CODE ##########

/**
 * @brief Displays the starting graphics
 */
void MazeScene::renderHints(AppContext &ctx, int8_t textY)
{
    ctx.gfx.clear();
    ctx.gfx.setCursor(0, textY);
    ctx.gfx.setTextColor(kPlayerColor);
    ctx.gfx.println(textPlayer);
    ctx.gfx.setTextColor(kWallColor);
    ctx.gfx.println(textWall);
    ctx.gfx.setTextColor(kStartColor);
    ctx.gfx.println(textStart);
    ctx.gfx.setTextColor(kFinishColor);
    ctx.gfx.println(textFinish);
    ctx.gfx.setTextColor(kSolutionColor);
    ctx.gfx.println(textHint);
    ctx.gfx.setTextColor(kFoodColor);
    ctx.gfx.println(textFood);
    ctx.gfx.setTextColor(kTimeColor);
    ctx.gfx.println(textTime);
}

void MazeScene::setStage(AppContext &ctx, Stage newStage)
{
    switch (newStage)
    {
    case Intro:
        textY = kStartTextY;
        break;
    case Game:
        buildMaze();
        setMazeEndpoints();
        break;
    case End:

        break;
    }
    startTime = ctx.time.nowMs();
    lastUpdateTime = startTime;
    stage = newStage;
}

void MazeScene::setup(AppContext &ctx)
{
    setStage(ctx, Intro);
}

void MazeScene::loop(AppContext &ctx)
{
    millis_t now = ctx.time.nowMs();
    switch (stage)
    {
    case (Intro):
        if ((textY > kStartTextYStop)) // display hints until they're off-screen
        {
            renderHints(ctx, textY);
            // scroll only at the given rate
            if ((now - lastUpdateTime) > kStartTextStepRate)
            {
                --textY;
                lastUpdateTime = now;
            }
        }
        else
        {
            setStage(ctx, Game);
        }
        break;
    case (Game):
        sampleStrobedDirection(ctx, inputDir, lastUpdateTime);

        if (inputDir != Maze::Direction::None)
            movePlayer(ctx);

        colorMaze();
        colorStart();
        colorFinish();
        colorPlayer();
        displayMaze(ctx);
        displayTimer(ctx);
        break;
    case (End):
        // TODO implement
        break;
    }
}

// ########## MAZE CODE ##########

/**
 * @brief Comparator used by priority queue to compare two nodes by their value
 *
 * @param u node 1
 * @param v node 2
 * @return true if node 1's value is greater than node 2's
 * @return false otherwise
 */
bool Maze::node::compare(node *u, node *v)
{
    return u && v && u->value > v->value;
}

namespace std
{
    template <>
    struct hash<Maze::node>
    {
        std::size_t operator()(const Maze::node &p) const
        {
            return p.pos;
        }
    };
}

/**
 * @brief Builds the adjacency graph for the maze
 *
 */
void MazeScene::buildAdjacencyGraph(Maze::graph &adj_g)
{
    for (byte r = 0; r < Maze::kMazeHeight; r++)
    {
        for (byte c = 0; c < Maze::kMazeWidth; c++)
        {
            Maze::coord p = Maze::encode(c, r);
            Maze::node &v = adj_g.vertices[p];
            v.pos = p; // set the pos of the node

            // connect to top node
            if (r > 0)
                adj_g.insertEdge(p, Maze::encode(c, r - 1), Helpers::random(Maze::kMaxEdges));
            // connect to left node
            if (c > 0)
                adj_g.insertEdge(p, Maze::encode(c - 1, r), Helpers::random(Maze::kMaxEdges));
        }
    }
}

/**
 * @brief Apply BFS to find furthest node from src
 */
Maze::coord MazeScene::getFurthestIndex(Maze::coord src)
{
    bool visited[maze_g.size];
    int8_t distance[maze_g.size];
    for (Maze::coord i = 0; i < maze_g.size; ++i)
    {
        visited[i] = false; // mark all nodes as visited
        distance[i] = -1;   // mark all distances as -1
    }
    std::queue<Maze::coord> queue{};
    queue.push(src);
    visited[src] = true; // mark first node as visited
    distance[src] = 0;   // distance from src-src is 0

    Maze::coord v;
    while (!queue.empty())
    {
        v = queue.front();
        queue.pop();
        Maze::node &curr = maze_g.vertices[v];

        for (Maze::direction_t i = 0; i < Maze::kMaxNeighbors; i++)
        {
            Maze::coord n = curr.pos_relative(i);
            if (n == Maze::coord(Maze::Direction::None))
                continue; // skip, the direction is invalid because neighbor is on border

            if (curr.weights[i] == Maze::Direction::None)
                continue; // there's no neighbor in this direction so skip
            if (visited[n])
                continue; // if we visited this neighbor node already, skip

            visited[n] = true;
            distance[n] = distance[v] + 1;
            queue.push(n);
        }
    }

    int8_t maxDistance = 0;
    Maze::coord furthestIdx = src;
    for (Maze::coord i = 0; i < maze_g.size; ++i)
    {
        if (distance[i] > maxDistance)
        {
            maxDistance = distance[i];
            furthestIdx = i;
        }
    }

    return furthestIdx;
}

/**
 * @brief Set the end points of the maze object
 *
 * Uses BFS to find the longest path in the maze
 */
void MazeScene::setMazeEndpoints()
{
    // choose furthest from top left corner for finish
    endNode = &maze_g.vertices[getFurthestIndex(0)];
    // choose furthest from finish for start
    startNode = &maze_g.vertices[getFurthestIndex(endNode->pos)];
    Maze::maze_t x = Maze::getX(startNode->pos);
    Maze::maze_t y = Maze::getY(startNode->pos);
    playerX = Maze::toMatrix(x);
    playerY = Maze::toMatrix(y);
}

// Custom fixed-capacity heap to save memory
struct NodePtrHeap
{
    Maze::node *a[Maze::graph::size];
    uint16_t n = 0;
    static inline bool less(Maze::node *u, Maze::node *v) { return u->value > v->value; } // min by value
    void push(Maze::node *x)
    {
        a[n] = x;
        uint16_t i = n++;
        while (i)
        {
            uint16_t p = (i - 1) / 2;
            if (!less(a[p], a[i]))
                break;
            auto t = a[p];
            a[p] = a[i];
            a[i] = t;
            i = p;
        }
    }
    Maze::node *top() const { return a[0]; }
    void pop()
    {
        if (!n)
            return;
        a[0] = a[--n];
        uint16_t i = 0;
        for (;;)
        {
            uint16_t l = 2 * i + 1, r = l + 1, m = i;
            if (l < n && less(a[m], a[l]))
                m = l;
            if (r < n && less(a[m], a[r]))
                m = r;
            if (m == i)
                break;
            auto t = a[i];
            a[i] = a[m];
            a[m] = t;
            i = m;
        }
    }
    bool empty() const { return n == 0; }
};

/**
 * @brief Builds the maze graph using Prim's algorithm
 *
 */
void MazeScene::buildMaze()
{
    // 1) Local adjacency graph (auto-freed on return)
    Maze::graph adj_g;
    buildAdjacencyGraph(adj_g);

    // 2) Init final maze graph nodes
    for (Maze::coord p = 0; p < Maze::graph::size; p++)
        maze_g.vertices[p].pos = p;

    // initialize the queue of vertices not in the maze
    NodePtrHeap pq;
    pq.push(&adj_g.vertices[0]); // build from first node

    while (!pq.empty()) // until the maze has all vertices
    {
        Maze::node *v = pq.top(); // take the vertex with the cheapest edge
        pq.pop();

        v->used = true;                                  // mark v as a used vertex in the maze
        if (v->id != Maze::coord(Maze::Direction::None)) // if v touches the maze, add cheapest neighboring edge to maze
            maze_g.insertEdge(v->pos, v->pos_relative(v->id), v->value);

        // loop through outgoing edges of vertex
        for (byte i = 0; i < Maze::kMaxNeighbors; i++)
        {
            int e = v->weights[i];          // edge cost
            if (e == Maze::Direction::None) // if edge doesn't exist
                continue;

            Maze::coord n = v->pos_relative(i); // get neighbor index
            Maze::node *w = &adj_g.vertices[n]; // neighbor node
            if (!w->used && e < w->value)       // if a new neighboring cheapest edge is found
            {
                // update neighbor's cheapest edge
                w->value = v->weights[i];
                w->id = ((*v) - (*w));
                pq.push(w);
            }
        }
    }
}

/**
 * @brief Colors the start of the maze
 *
 */
void MazeScene::colorStart()
{
    // Color special nodes
    Maze::maze_t x, y;
    x = Maze::getX(startNode->pos);
    y = Maze::getY(startNode->pos);
    grid[Maze::toMatrix(y)][Maze::toMatrix(x)] = PaletteIndex::Start;
}

/**
 * @brief Colors the finish of the maze
 *
 */
void MazeScene::colorFinish()
{
    // Color special nodes
    byte x, y;
    x = Maze::getX(endNode->pos);
    y = Maze::getY(endNode->pos);
    grid[Maze::toMatrix(y)][Maze::toMatrix(x)] = PaletteIndex::Finish;
}

/**
 * @brief Color the maze and walls
 *
 */
void MazeScene::colorMaze()
{
    for (Maze::matrix_t r = 0; r < MATRIX_HEIGHT; r++)
    {
        for (Maze::matrix_t c = 0; c < MATRIX_WIDTH; c++)
        {
            grid[r][c] = PaletteIndex::Wall; // color wall
        }
    }
    Maze::maze_t x, y;
    for (Maze::coord p = 0; p < maze_g.size; p++)
    {
        x = Maze::getX(p);
        y = Maze::getY(p);
        Maze::matrix_t r = Maze::toMatrix(y);
        Maze::matrix_t c = Maze::toMatrix(x);
        grid[r][c] = PaletteIndex::None; // color the vertex node

        // color the edge nodes
        Maze::maze_t x2, y2;
        Maze::node *v = &maze_g.vertices[p];
        for (Maze::coord i = 0; i < Maze::kMaxNeighbors; i++)
        {
            if (v->weights[i] == Maze::Direction::None) // if edge doesn't exist
                continue;

            Maze::node *u = &maze_g.vertices[v->pos_relative(i)];
            x2 = Maze::getX(u->pos);
            y2 = Maze::getY(u->pos);
            r = Maze::interpolate(y, y2);
            c = Maze::interpolate(x, x2);
            grid[r][c] = PaletteIndex::None;
        }
    }
}

// MazeScene.cpp
// Choose a 4-way direction with strobing and variable repeat rate.
// - strobe=true resets inputDir at the start of a strobe window (like your sketch)
// - Uses normalized stick {x,y} in [-1, +1] from InputProvider
Maze::Direction MazeScene::sampleStrobedDirection(AppContext &ctx, Maze::Direction &ioDir, millis_t &ioLastTimeMs)
{
    InputState s = ctx.input.state();
    millis_t nowMs = ctx.time.nowMs();
    // Reset the instantaneous direction
    ioDir = Maze::Direction::None;

    const float dx = s.x;
    const float dy = s.y;

    // If outside deadband, consider a move on the dominant axis
    if (std::fabs(dx) > kInputBuffer || std::fabs(dy) > kInputBuffer)
    {
        // Default repeat delay, faster near extremes
        millis_t inputDelay = kDefaultInputDelayMs;

        // Decide axis and candidate direction
        Maze::Direction cand = Maze::Direction::None;
        if (std::fabs(dx) >= std::fabs(dy))
        {
            if (dx > kInputBuffer)
                cand = Maze::Direction::Right;
            if (dx < -kInputBuffer)
                cand = Maze::Direction::Left;
            if (std::fabs(dx) >= kFastInputThreshold)
                inputDelay = kFastInputDelayMs;
        }
        else
        {
            if (dy > kInputBuffer)
                cand = Maze::Direction::Down;
            if (dy < -kInputBuffer)
                cand = Maze::Direction::Up;
            if (std::fabs(dy) >= kFastInputThreshold)
                inputDelay = kFastInputDelayMs;
        }

        // Gate by repeat timer, like (currentTime - lastInputTime > inputDelay)
        if (cand != Maze::Direction::None &&
            (nowMs - ioLastTimeMs) > inputDelay)
        {
            ioDir = cand;
        }
    }

    // Update last time only when a step will be consumed this frame
    if (ioDir != Maze::Direction::None)
    {
        ioLastTimeMs = nowMs;
    }

    return ioDir;
}

/**
 * @brief Move the position of the player in the given direction
 *
 */
void MazeScene::movePlayer(AppContext &ctx)
{
    int8_t dx, dy;
    dx = 0;
    dy = 0;
    Maze::maze_t x, y;
    switch (inputDir)
    {
    case Maze::Direction::Up:
        dy = -1;
        break;
    case Maze::Direction::Down:
        dy = 1;
        break;
    case Maze::Direction::Left:
        dx = -1;
        break;
    case Maze::Direction::Right:
        dx = 1;
        break;
    default:
        return;
    }
    x = Helpers::clamp(playerX + dx, 1, MATRIX_WIDTH - 2);
    y = Helpers::clamp(playerY + dy, 1, MATRIX_HEIGHT - 2);
    if (grid[y][x] != PaletteIndex::Wall)
    {
        playerX = x;
        playerY = y;
    }
}

/**
 * @brief Color the player position
 *
 */
void MazeScene::colorPlayer()
{
    grid[playerY][playerX] = PaletteIndex::Player;
    // brightenSurroundings(); // TODO implement
}

/**
 * @brief Whether the pixel is part of the border
 *
 * @param r
 * @param c
 * @return true
 * @return false
 */
bool MazeScene::isBorder(Maze::matrix_t r, Maze::matrix_t c)
{
    return r == 0 || r == Maze::toMatrix(Maze::kMazeHeight) - 1 || c == 0 || c == Maze::toMatrix(Maze::kMazeWidth) - 1;
}

/**
 * @brief Display the maze on the matrix
 *
 */
void MazeScene::displayMaze(AppContext &ctx)
{
    Color333 color;
    // used for centering
    Maze::matrix_t rowOffset;
    Maze::matrix_t colOffset;
    for (Maze::maze_t r = 0; r < Maze::toMatrix(Maze::kMazeHeight); r++)
    {
        for (Maze::maze_t c = 0; c < Maze::toMatrix(Maze::kMazeWidth); c++)
        {
            color = palette(grid[r][c]);
            rowOffset = (MATRIX_HEIGHT - Maze::toMatrix(Maze::kMazeHeight)) / 2;
            colOffset = (MATRIX_WIDTH - Maze::toMatrix(Maze::kMazeWidth)) / 2;
            ctx.gfx.drawPixel(c + colOffset, r + rowOffset, color);
        }
    }
}

/**
 * @brief Display the time left in the game
 *
 */
void MazeScene::displayTimer(AppContext &ctx)
{
    using TimerCount_t = uint8_t;
    TimerCount_t pixelsPassed = ((kTimerPixels) * (ctx.time.nowMs() - startTime)) / kGameDefaultDuration;
    Maze::matrix_t r, c;

    if (pixelsPassed >= kTimerPixels)
    {
        endGame(ctx);
        return;
    }

    // draw time track
    for (TimerCount_t i = 1; i <= kTimerPixels; i++)
    {
        if (i < MATRIX_HEIGHT)
        {
            r = i;
            c = MATRIX_WIDTH;
        }
        else
        {
            r = MATRIX_HEIGHT;
            c = MATRIX_WIDTH - (i - MATRIX_HEIGHT);
        }

        if (i <= pixelsPassed)
            grid[r - 1][c - 1] = PaletteIndex::None;
        else
            grid[r - 1][c - 1] = PaletteIndex::Time;

        ctx.gfx.drawPixel(c - 1, r - 1, palette(grid[r - 1][c - 1]));
    }
}

/**
 * @brief End the game
 *
 */
void MazeScene::endGame(AppContext &ctx)
{
    ctx.bus->toMenu(); // TODO implement
}
