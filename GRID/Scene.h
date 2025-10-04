#ifndef SCENE_H
#define SCENE_H

#include "AppContext.h"
#include "helpers.h"

/**
 * @brief Preferences for scene timing
 */
struct SceneTimingPrefs
{
  double targetHz;
};

/**
 * @brief Describes a scene of the game with explicit lifecycle
 *
 */
struct Scene
{
  virtual ~Scene() = default;
  // Return the default FPS
  virtual SceneTimingPrefs timingPrefs() const { return {60.0}; };
  // Called once when the scene is switched to
  virtual void setup(AppContext &ctx) = 0;
  // dt is in milliseconds
  virtual void loop(AppContext &cfx) = 0;
};

#endif
