#pragma once

#include <memory>
#include "mesh.hh"
#include "scene.hh"

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

class Physics_Manager {
public:
  
  std::shared_ptr<Scene> m_active_scene = nullptr;
  bool m_phys_boxes_initialized = false;
  
  Physics_Manager(std::shared_ptr<Scene> set_scene);

  void handle_scene_physics();

  Mesh create_collision_box_mesh(const AABB& box);

  AABB compute_world_space_aabb(const Mesh& mesh, const glm::mat4& transform);

  void calculate_phys_boxes();
  
};
