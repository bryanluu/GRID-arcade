#ifndef SNAKE_SCENE_H
#define SNAKE_SCENE_H

#include "Scene.h"
#include "Colors.h"

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
    enum Direction
    {
        Up,
        Down,
        Left,
        Right
    } direction_;

public:
    static constexpr int kInitialLength = 3;

    Snake(int startX, int startY);
    ~Snake();
    // void move();
    // void grow();
    // bool checkCollision(int gridWidth, int gridHeight) const;
    int getHeadX() const;
    int getHeadY() const;
    Direction getDirection() const;
    void setDirection(Direction dir);
    int getLength() const;
    void draw(class Matrix32 &gfx, Color333 color) const;
};

class SnakeScene : public Scene
{
    Snake snake_;

public:
    SnakeScene();

    void setup(AppContext &ctx) override;

    void loop(AppContext &ctx) override;

    SceneKind kind() const override { return SceneKind::Snake; }
    const char *label() const override { return "Snake"; }
};

#endif // SNAKE_SCENE_H
