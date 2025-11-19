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
    if (collided_)
        return; // do not move if collided

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
    int segmentsMoved = 0;
    Node *newHead = new Node(newX, newY);

    collided_ = checkCollision(newHead);
    if (collided_)
    {
        delete newHead;
        return; // collision detected, do not move
    }

    newHead->next = head_;
    head_ = newHead;
    segmentsMoved++;
    // Remove tail
    Node *current = head_;
    while (current->next != tail_)
    {
        current = current->next;
        segmentsMoved++;
    }
    if (segmentsMoved >= length_)
    {
        tail_ = current;
        delete tail_->next;
        tail_->next = nullptr;
    }
}

void Snake::grow()
{
    length_ += 1;
    // Do not remove tail on next move
}

bool Snake::checkCollision(Node *newHead) const
{
    // Check wall collision
    if (newHead->x < 0 || newHead->x >= MATRIX_WIDTH || newHead->y < 0 || newHead->y >= MATRIX_HEIGHT)
        return true;

    // Check self-collision
    Node *current = newHead->next;
    while (current)
    {
        if (current->x == newHead->x && current->y == newHead->y)
            return true;
        current = current->next;
    }
    return false;
}

void Snake::draw(class Matrix32 &gfx, Color333 color, std::bitset<MATRIX_WIDTH * MATRIX_HEIGHT> &occupied) const
{
    Node *current = head_;
    while (current)
    {
        if (collided_)
            gfx.setSafe(current->x, current->y, Colors::Muted::Yellow); // Indicate collision
        else
        {
            gfx.setSafe(current->x, current->y, color);
            occupied.set(current->y * MATRIX_WIDTH + current->x);
        }
        current = current->next;
    }
}

SnakeScene::SnakeScene() : snake_(Snake::kInitialLength - 1 + Helpers::random(MATRIX_WIDTH - Snake::kInitialLength),
                                  Snake::kInitialLength - 1 + Helpers::random(MATRIX_HEIGHT - Snake::kInitialLength)) // Randomly place snake
{
}

void SnakeScene::setup(AppContext &ctx)
{
    placeFood();
}

void SnakeScene::placeFood()
{
    // Randomly place food on available spaces
    int available = (~occupied_).count();
    if (available == 0)
        return; // no space available

    int target = Helpers::random(available);
    int x = 0;      // index in available spaces
    int i = 0;      // index in bitset
    PixelMap p = 1; // target in bitset
    while (x < target || (occupied_ & p).any())
    {
        if ((~occupied_ & p).any()) // if this space is free, move to next available
            x++;
        i++;
        p <<= 1;
    } // p now points to the target free space

    foodX_ = i % MATRIX_WIDTH;
    foodY_ = i / MATRIX_WIDTH;
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

    // Check for food consumption
    if (snake_.getHeadX() == foodX_ && snake_.getHeadY() == foodY_)
    {
        snake_.grow();
        placeFood();
    }

    // Draw snake
    snake_.draw(ctx.gfx, Colors::Muted::Green, occupied_);

    // Draw food
    ctx.gfx.setSafe(foodX_, foodY_, Colors::Muted::Red);
}
