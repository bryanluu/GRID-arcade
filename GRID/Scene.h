#ifndef SCENE_H
#define SCENE_H

#include "AppContext.h"
#include "Helpers.h"
#include "Timing.h"
#include <cmath>
#include <limits>

/**
 * @brief Describes a scene of the game with explicit lifecycle
 *
 */
struct Scene
{
  enum class SceneKind : uint8_t
  {
    Start,
    Menu,
    Boids,
    Calibration,
    Maze,
    Test,
    SaveScore,
    Snake
  };

  virtual ~Scene() = default;

  virtual SceneKind kind() const = 0;
  virtual const char *label() const = 0;
  // Override to specify a preferred targetHz for the scene
  // Return NaN to use default (60Hz if not set otherwise)
  virtual SceneTimingPrefs timingPrefs() const { return SceneTimingPrefs(std::numeric_limits<float>::quiet_NaN()); };
  // Called once when the scene is switched to
  virtual void setup(AppContext &ctx) = 0;
  // dt is in milliseconds
  virtual void loop(AppContext &cfx) = 0;
};

#endif
