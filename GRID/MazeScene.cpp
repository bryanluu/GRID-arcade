#include "MazeScene.h"
#include "Helpers.h"
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
void MazeScene::renderHints(AppContext &ctx)
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

void MazeScene::setup(AppContext &ctx)
{
    // TODO uncomment to show Hints
    // startTime = ctx.time.nowMs();
    // textY = 5;

    // // show Hint text before game starts
    // ctx.gfx.setImmediate(false);
    // while (--textY > kStartLoopStopY)
    // {
    //     renderHints(ctx);
    //     ctx.gfx.show();
    //     ctx.time.sleep(kStartLoopDelay);
    // }
    // ctx.gfx.setImmediate(true);

    buildMaze();
}

void MazeScene::loop(AppContext &ctx)
{
    colorMaze();
    displayMaze(ctx);
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
 * @brief Set the end points of the maze object
 *
 */
void MazeScene::setMazeEndpoints()
{
    // set endpoints
    startNode = &maze_g.vertices[0];
    endNode = &maze_g.vertices[Maze::graph::size - 1];
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
    {
        maze_g.vertices[p].pos = p;
        maze_g.vertices[p].used = false;
        maze_g.vertices[p].value = INT16_MAX;
        for (uint8_t i = 0; i < Maze::kMaxNeighbors; ++i)
            maze_g.vertices[p].weights[i] = -1;
    }

    setMazeEndpoints();

    // initialize the queue of vertices not in the maze
    NodePtrHeap pq;
    pq.push(&adj_g.vertices[startNode->pos]); // build from start node

    while (!pq.empty()) // until the maze has all vertices
    {
        Maze::node *v = pq.top(); // take the vertex with the cheapest edge
        pq.pop();

        v->used = true;                     // mark v as a used vertex in the maze
        if (v->id != Maze::Direction::None) // if v touches the maze, add cheapest neighboring edge to maze
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
 * @brief Color the maze and walls
 *
 */
void MazeScene::colorMaze()
{
    for (Maze::matrix_t r = 0; r < MATRIX_HEIGHT; r++)
    {
        for (Maze::matrix_t c = 0; c < MATRIX_WIDTH; c++)
        {
            grid[r][c] = MazeScene::kWallColor; // color wall
        }
    }
    Maze::maze_t x, y;
    for (Maze::coord p = 0; p < maze_g.size; p++)
    {
        x = Maze::getX(p);
        y = Maze::getY(p);
        Maze::matrix_t r = Maze::toMatrix(y);
        Maze::matrix_t c = Maze::toMatrix(x);
        grid[r][c] = Colors::Black; // color the vertex node

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
            grid[r][c] = Colors::Black;
        }
    }
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
            color = grid[r][c];
            rowOffset = (MATRIX_HEIGHT - Maze::toMatrix(Maze::kMazeHeight)) / 2;
            colOffset = (MATRIX_WIDTH - Maze::toMatrix(Maze::kMazeWidth)) / 2;
            ctx.gfx.drawPixel(c + colOffset, r + rowOffset, color);
        }
    }
}
