#ifndef SCENE_H
#define SCENE_H

#include "App.h"
#include <string>

/**
 * @brief Describes a scene of the game
 * 
 */
 class Scene
 {
   public:
     static Scene * currentScene; // which scene is running
     static App * app;
     std::string name{};
 
     Scene();
 
     /**
      * @brief Any initializing actions go here
      * 
      */
     virtual void start();
 
     /**
      * @brief Core code during scene goes here
      * 
      */
     virtual void run();
 };

#endif
