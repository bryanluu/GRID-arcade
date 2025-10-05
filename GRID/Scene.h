#ifndef SCENE_H
#define SCENE_H

#include "AppContext.h"
#include "helpers.h"
#include "Timing.h"

/**
 * @brief Describes a scene of the game with explicit lifecycle
 *
 */
struct Scene
{
  virtual ~Scene() = default;
  // Override to specify a preferred targetHz for the scene
  // Return NaN to use default (60Hz if not set otherwise)
  virtual SceneTimingPrefs timingPrefs() const { return {NAN}; };
  // Called once when the scene is switched to
  virtual void setup(AppContext &ctx) = 0;
  // dt is in milliseconds
  virtual void loop(AppContext &cfx) = 0;
};

#endif
