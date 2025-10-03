#ifndef SCENE_H
#define SCENE_H

#include "AppContext.h"
#include "helpers.h"

/**
 * @brief Describes a scene of the game with explicit lifecycle
 *
 */
struct Scene
{
  virtual ~Scene() = default;
  // Called once when the scene is switched to
  virtual void setup(AppContext &ctx) = 0;
  // dt is in milliseconds
  virtual void loop(AppContext &cfx, millis_t dt) = 0;
};

#endif
