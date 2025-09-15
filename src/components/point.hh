#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

//TODO make this shit not be declared here :-) 
struct Physics_properties {

  bool is_physics_object = false;
  bool stationary = true;
  bool fixed = false;

  // accumulated force, current velocity, current acceleration
  glm::vec3 force = {0,0,0};
  glm::vec3 velocity = {0,0,0};
  glm::vec3 acceleration = {0,0,0};

  float inverse_mass = 1.0f;

  //why tf is this valid
  void add_force(glm::vec3 to_add) {force += to_add;};
  void add_force(float x, float y, float z) { force.x += x; force.y += y; force.z += z; };
  
};

class Point {

  bool model_matrix_deprecated = true;

  glm::mat4 m_model_matrix_buffer;

  float m_pos_x = 0, m_pos_y = 0, m_pos_z = 0;
  
public:

  glm::mat4 get_model_matrix();
  
  void set_position(float x ,float y, float z);
  
  void change_position(float x ,float y, float z);
  void change_position(glm::vec3);
  
  glm::vec3 get_position();
  
  Point(float x, float y, float z);
  
  Physics_properties phys_props;

  unsigned int VAO_id;

  unsigned int VBO_vertices;

};
