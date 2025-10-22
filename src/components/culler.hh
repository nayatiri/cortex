#pragma once

#include "scene.hh"
class Culler {
public:

  std::shared_ptr<Scene> m_active_scene = nullptr;

  void cull_scene();
  void update_active_scene(std::shared_ptr<Scene> new_scene);
  
  Culler(std::shared_ptr<Scene> new_scene);
  
};
