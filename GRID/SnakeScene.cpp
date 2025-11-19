#include "SnakeScene.h"
#include "Colors.h"
#include "Helpers.h"

Snake::Snake(int startX, int startY)
    : length_(3), direction_(Right)
{
    head_ = new Node(startX, startY);
    Node *second = new Node(startX - 1, startY);
    Node *third = new Node(startX - 2, startY);
    head_->next = second;
    second->next = third;
    tail_ = third;
}

Snake::~Snake()
{
    Node *current = head_;
    while (current)
    {
        Node *next = current->next;
        delete current;
        current = next;
    }
}

int Snake::getHeadX() const
{
    return head_->x;
}

int Snake::getHeadY() const
{
    return head_->y;
}

Snake::Direction Snake::getDirection() const
{
    return direction_;
}

void Snake::setDirection(Direction dir)
{
    direction_ = dir;
}

int Snake::getLength() const
{
    return length_;
}

void Snake::draw(class Matrix32 &gfx, Color333 color) const
{
    Node *current = head_;
    while (current)
    {
        gfx.setSafe(current->x, current->y, color);
        current = current->next;
    }
}

SnakeScene::SnakeScene() : snake_(Snake::kInitialLength - 1 + Helpers::random(MATRIX_WIDTH - Snake::kInitialLength),
                                  Snake::kInitialLength - 1 + Helpers::random(MATRIX_HEIGHT - Snake::kInitialLength)) // Randomly place snake
{
}

void SnakeScene::setup(AppContext &ctx)
{
}

void SnakeScene::loop(AppContext &ctx)
{
    // Draw snake
    snake_.draw(ctx.gfx, Colors::Muted::Green);
}
