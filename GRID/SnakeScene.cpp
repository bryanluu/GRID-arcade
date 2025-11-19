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
    // Prevent reversing direction
    if ((direction_ == Up && dir == Down) ||
        (direction_ == Down && dir == Up) ||
        (direction_ == Left && dir == Right) ||
        (direction_ == Right && dir == Left))
    {
        return;
    }

    direction_ = dir;
}

int Snake::getLength() const
{
    return length_;
}

void Snake::move()
{
    int newX = head_->x;
    int newY = head_->y;

    switch (direction_)
    {
    case Up:
        newY -= 1;
        break;
    case Down:
        newY += 1;
        break;
    case Left:
        newX -= 1;
        break;
    case Right:
        newX += 1;
        break;
    }

    // Move all nodes forward
    Node *newHead = new Node(newX, newY);
    newHead->next = head_;
    head_ = newHead;
    // Remove tail
    Node *current = head_;
    while (current->next != tail_)
    {
        current = current->next;
    }
    tail_ = current;
    delete tail_->next;
    tail_->next = nullptr;
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
    ctx.gfx.clear();

    // Control snake direction based on input
    InputState input = ctx.input.state();
    if (input.x < -0.5f)
        snake_.setDirection(Snake::Direction::Left);
    else if (input.x > 0.5f)
        snake_.setDirection(Snake::Direction::Right);
    else if (input.y < -0.5f)
        snake_.setDirection(Snake::Direction::Up);
    else if (input.y > 0.5f)
        snake_.setDirection(Snake::Direction::Down);
    snake_.move();

    // Draw snake
    snake_.draw(ctx.gfx, Colors::Muted::Green);
}
