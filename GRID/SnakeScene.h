#ifndef SNAKE_SCENE_H
#define SNAKE_SCENE_H

#include "Scene.h"
#include "Colors.h"
#include <bitset>

class Snake
{
    struct Node
    {
        int x;
        int y;
        Node *next;
        Node(int x, int y) : x(x), y(y), next(nullptr) {}
    };

    Node *head_;
    Node *tail_;
    int length_;
    bool collided_ = false;
    bool checkCollision(Node *newHead) const;

public:
    static constexpr int kInitialLength = 3;

    enum Direction
    {
        Up,
        Down,
        Left,
        Right
    };

    Snake(int startX, int startY);
    ~Snake();
    void move();
    void grow();
    int getHeadX() const;
    int getHeadY() const;
    Direction getDirection() const;
    void setDirection(Direction dir);
    int getLength() const;
    bool hasCollided() const { return collided_; }
    void draw(class Matrix32 &gfx, Color333 color, std::bitset<MATRIX_WIDTH * MATRIX_HEIGHT> &occupied) const;

private:
    Direction direction_;
};

class SnakeScene : public Scene
{
    Snake snake_;
    int foodX_;
    int foodY_;

    void placeFood();
    using PixelMap = std::bitset<MATRIX_WIDTH * MATRIX_HEIGHT>;
    PixelMap occupied_;

public:
    SnakeScene();

    SceneTimingPrefs timingPrefs() const override
    {
        return SceneTimingPrefs(10.0f); // slower update rate for snake
    }

    void setup(AppContext &ctx) override;

    void loop(AppContext &ctx) override;

    SceneKind kind() const override { return SceneKind::Snake; }
    const char *label() const override { return "Snake"; }
};

#endif // SNAKE_SCENE_H
