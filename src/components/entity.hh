#pragma once

#include "animation.hh"
#include "mesh.hh"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// stdlib
#include <vector>

enum Entity_types {

  Entity_3D,
  Entity_Point
  
};

class Entity {
private:
  glm::mat4 m_model_matrix_buffer;
  
public:
  bool is_initialized();

  Entity_types entity_type = Entity_3D;
  
  bool model_matrix_deprecated = true;
  float m_pos_x = 0, m_pos_y = 0, m_pos_z = 0;
  float m_rot_x = 0, m_rot_y = 0, m_rot_z = 0;
  glm::mat4 get_model_matrix();

  void set_position(float x ,float y, float z);
  void change_position(float x ,float y, float z);

  void set_rotation (float rx ,float ry, float rz);
  void change_rotation (float rx ,float ry, float rz);

  std::shared_ptr<Shader> point_cloud_shader = nullptr;
  
  std::vector<Mesh> m_mesh = {};

  std::vector<animation>* m_animation_table;
  
};
