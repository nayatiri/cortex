#pragma once

#include "scene.hh"
#include <memory>
class Visibility_Manager {
public:

  int get_sign_of_point_relative_plane(glm::vec3 plp1, glm::vec3 plp2, glm::vec3 plp3, glm::vec3 point);

  inline float fastDeterminant(const glm::mat3& m);
  
  std::shared_ptr<Scene> m_active_scene = nullptr;
  
};
