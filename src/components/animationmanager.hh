#pragma once

#include <memory>
#include <stdlib.h>
#include "scene.hh"

class Animation_Manager {
public:

  void handle_scene_animations(float m_application_current_time);

  Animation_Manager(std::shared_ptr<Scene> m_active_scene);
  
  std::shared_ptr<Scene> m_active_scene = nullptr;

  };
