#pragma once

#include <memory>
#include "force_generator.hh"
#include "mesh.hh"
#include "scene.hh"

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

class Physics_Manager {

private:
  void handle_scene_physics_diy();
  bool initialize_force_generators();
  void handle_scene_physics_book();
  bool check_inside_AABB(Mesh &mesh, glm::vec3 check_position);
  
public:
  
  std::shared_ptr<Scene> m_active_scene = nullptr;
  
  bool m_phys_boxes_initialized = false;
  bool m_force_generators_initialized = false;
  
  Physics_Manager(std::shared_ptr<Scene> set_scene);

  void handle_scene_physics();

  AABB_Box create_collision_box_mesh(const AABB& box);

  AABB compute_world_space_aabb(Mesh& mesh, const glm::mat4& transform);

  void calculate_phys_boxes();

  std::vector<Force_generator> force_generators;
  
};
