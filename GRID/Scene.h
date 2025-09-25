#ifndef SCENE_H
#define SCENE_H

#include <string>

/**
 * @brief Describes a scene of the game
 * 
 */
 class Scene
 {
   public:
     static Scene * s_currentScene; // which scene is running
     std::string m_name{};
 
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
