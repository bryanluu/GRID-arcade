#ifndef SCENE_H
#define SCENE_H

#include "Matrix32.h"
#include "helpers.h"

/**
 * @brief Describes a scene of the game with explicit lifecycle
 *
 */
struct Scene
{
  virtual ~Scene() = default;
  // Called once when the scene is switched to
  virtual void setup(Matrix32 &gfx) = 0;
  // dt is in milliseconds
  virtual void loop(Matrix32 &gfx, millis_t dt) = 0;
};

#endif
