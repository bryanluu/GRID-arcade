#ifndef BOIDS_SCENE_H
#define BOIDS_SCENE_H

#include "Scene.h"
#include "Vector.h"

// Tunable parameters
#define N_BOIDS 15             // number of boids to simulate
#define BOID_SIZE 1            // how big are the boids?
#define MIN_SPEED 0.4          // min speed of a boid
#define MAX_SPEED 1            // max speed of a boid
#define MARGIN 4               // margins at which to start turning
#define TURN_FACTOR 0.15       // how quickly do boids avoid edges?
#define PROTECTED_RANGE 1.5    // the range at which boids avoid others
#define AVOID_FACTOR 0.05      // how quickly do boids avoid each other?
#define VISIBLE_RANGE 5        // follow others within this range
#define MATCHING_FACTOR 0.06   // how quickly boids should follow flock?
#define CENTERING_FACTOR 0.005 // how closely do boids follow flock?
#define LONELY_LIMIT 2         // below how many separated boids is considered 'lonely'?

// Boid drawing
#define DEFAULT_COLOR (Color333{4, 4, 7}) // color of a boid by default
#define DANGER_COLOR (Color333{7, 2, 2})  // color of a boid in danger
#define LONELY_COLOR (Color333{7, 7, 1})  // color of a boid that is lonely
#define SLOW_COLOR (Color333{1, 1, 7})    // color of a slow boid
#define PLAYER_COLOR (Color333{0, 4, 0})  // color of the player boid

struct Boid
{
  Vector position;
  Vector velocity;
  Vector closeness;
  Vector avgPosition;
  Vector avgVelocity;
  unsigned int neighbors;
};

class BoidsScene final : public Scene
{
  // creates flock
  Boid flock[N_BOIDS];
  int playerIndex = 0; // designate first boid as player Boids

  void placeBoid(Boid *boid);
  void constrainSpeed(Boid *boid);
  void controlPlayerBoid(AppContext &ctx);
  void avoidEdges(Boid *boid);
  void constrainPosition(Boid *boid);
  void followNeighbors(Boid *boid);
  void avoidOthers(Boid *boid);
  void flyWithFlock(Boid *boid, Boid *flock);
  void updateBoid(AppContext &ctx, Boid *boid, Boid *flock);
  void drawBoid(Matrix32 &gfx, Boid *boid);

public:
  SceneKind kind() const override { return SceneKind::Boids; }
  const char *label() const override { return "Boids"; }
  // matched measured hardware (~16.6 Hz)
  SceneTimingPrefs timingPrefs() const override { return SceneTimingPrefs(16.6); }
  void setup(AppContext &ctx) override;
  void loop(AppContext &ctx) override;
};

#endif
