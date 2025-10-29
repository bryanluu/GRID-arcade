#include "BoidsScene.h"
#include "App.h"
#include "Colors.h"
#include "Helpers.h"
#include "Matrix32.h"
#include <cmath>

/*
    Place the given Boid somewhere randomly along the screen
 */
void BoidsScene::placeBoid(Boid *boid)
{
    boid->position.x = Helpers::random(MATRIX_WIDTH);
    boid->position.y = Helpers::random(MATRIX_HEIGHT);
    long choice;
    choice = Helpers::random(1000);
    boid->velocity.x = ((choice % 2) ? 1 : -1) * (0.5 * MIN_SPEED + (MAX_SPEED - MIN_SPEED) * (choice / 1000.0));
    choice = Helpers::random(1000);
    boid->velocity.y = ((choice % 2) ? 1 : -1) * (0.5 * MIN_SPEED + (MAX_SPEED - MIN_SPEED) * (choice / 1000.0));
}

/*
    Make Boid avoid screen edges
 */
void BoidsScene::avoidEdges(Boid *boid)
{
    if (boid->position.x < MARGIN)
        boid->velocity.x += TURN_FACTOR;
    if (boid->position.x > MATRIX_WIDTH - 1 - MARGIN)
        boid->velocity.x -= TURN_FACTOR;
    if (boid->position.y < MARGIN)
        boid->velocity.y += TURN_FACTOR;
    if (boid->position.y > MATRIX_HEIGHT - 1 - MARGIN)
        boid->velocity.y -= TURN_FACTOR;
}

/*
    Ensure Boid speed remains within range
 */
void BoidsScene::constrainSpeed(Boid *boid)
{
    double speed = sqrt(boid->velocity.x * boid->velocity.x + boid->velocity.y * boid->velocity.y);
    if (speed > MAX_SPEED)
    {
        boid->velocity.x = (boid->velocity.x * MAX_SPEED) / speed;
        boid->velocity.y = (boid->velocity.y * MAX_SPEED) / speed;
    }
    if (speed < MIN_SPEED)
    {
        boid->velocity.x = (boid->velocity.x * MIN_SPEED) / speed;
        boid->velocity.y = (boid->velocity.y * MIN_SPEED) / speed;
    }
}

/*
    Constrain Boid to always stay within screen
 */
void BoidsScene::constrainPosition(Boid *boid)
{
    boid->position.x = Helpers::clamp(boid->position.x, 0.0, double(MATRIX_WIDTH - 1));
    boid->position.y = Helpers::clamp(boid->position.y, 0.0, double(MATRIX_HEIGHT - 1));
}

/*
    Follow neighboring boids
 */
void BoidsScene::followNeighbors(Boid *boid)
{
    if (boid->neighbors > 0)
    {
        boid->avgPosition = multiply(boid->avgPosition, 1.0 / boid->neighbors);
        boid->avgVelocity = multiply(boid->avgVelocity, 1.0 / boid->neighbors);
        boid->velocity = add(boid->velocity, multiply(sub(boid->avgVelocity, boid->velocity), MATCHING_FACTOR));
        boid->velocity = add(boid->velocity, multiply(sub(boid->avgPosition, boid->position), CENTERING_FACTOR));
    }
}

/*
    Avoid other boids
 */
void BoidsScene::avoidOthers(Boid *boid)
{
    boid->velocity = add(boid->velocity, multiply(boid->closeness, AVOID_FACTOR));
}

/*
    Fly with the flock
 */
void BoidsScene::flyWithFlock(Boid *boid, Boid *flock)
{
    zero(&boid->closeness);
    zero(&boid->avgPosition);
    zero(&boid->avgVelocity);
    boid->neighbors = 0;
    for (int i = 0; i < N_BOIDS; i++)
    {
        Boid *other = (flock + i);
        if (boid == other)
            continue;

        Vector diff = sub(boid->position, other->position);
        double dist = length(diff);
        if (dist < PROTECTED_RANGE)
            boid->closeness = add(boid->closeness, diff);
        if (dist < VISIBLE_RANGE)
        {
            boid->avgPosition = add(boid->avgPosition, other->position);
            boid->avgVelocity = add(boid->avgVelocity, other->velocity);
            boid->neighbors++;
        }
    }
    avoidOthers(boid);
    followNeighbors(boid);
}

/*
    Controls the player boid to follow input
*/
void BoidsScene::controlPlayerBoid(AppContext &ctx)
{
    Boid &player = flock[playerIndex];
    InputState s = ctx.input.state();

    player.velocity = add(player.velocity, s.vec());
}

/*
    Update loop for indivual Boid
 */
void BoidsScene::updateBoid(AppContext &ctx, Boid *boid, Boid *flock)
{
    flyWithFlock(boid, flock);
    controlPlayerBoid(ctx);
    avoidEdges(boid);
    constrainSpeed(boid);
    boid->position = add(boid->position, boid->velocity);
    constrainPosition(boid);
}

/*
    Draw an individual Boid
*/
void BoidsScene::drawBoid(Matrix32 &gfx, Boid *boid)
{
    MatrixPosition x = round(boid->position.x);
    MatrixPosition y = round(boid->position.y);
    Color333 color = DEFAULT_COLOR;

    if (boid - flock == playerIndex)
    {
        color = PLAYER_COLOR;
    }
    else if ((x == 0) || (x == MATRIX_WIDTH - 1) || (y == 0) || (y == MATRIX_HEIGHT - 1)) // boid is too close to the wall
    {
        color = DANGER_COLOR;
    }
    else if (boid->neighbors < LONELY_LIMIT) // boid is not part of a flock
    {
        color = LONELY_COLOR;
    }
    else
    {
        double frac_speed_limit = abs(MAX_SPEED - length(boid->velocity)) / MAX_SPEED;
        if (frac_speed_limit > 0.25) // boid is too slow
            color = SLOW_COLOR;
    }

    gfx.drawPixel(x, y, color);
}

void BoidsScene::setup(AppContext &ctx)
{
    // position boids
    for (int i = 0; i < N_BOIDS; i++)
    {
        placeBoid(&flock[i]);
    }
}

void BoidsScene::loop(AppContext &ctx)
{
    // Clear background
    ctx.gfx.clear();

    // update flock
    for (int i = 0; i < N_BOIDS; i++)
    {
        Boid *boid = &flock[i];
        updateBoid(ctx, boid, flock);
        drawBoid(ctx.gfx, boid);
    }

    // show the frame
    ctx.gfx.show();
}
