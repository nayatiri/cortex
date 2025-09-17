#pragma once

#include <cmath>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

struct Physics_properties {

  bool is_physics_object = false;
  bool stationary = true;
  bool fixed = false;

  // accumulated force, current velocity, current acceleration
  glm::vec3 force = {0,0,0};
  glm::vec3 velocity = {0,0,0};
  glm::vec3 acceleration = {0,0,0};

  float inverse_mass = 2.0f;

  //TMP grav here. ForceGenerator ass to access from other force generator
  float gravity = -9.81f;

  float radius = 0.2f;
  
  //why tf is this valid
  void add_force(glm::vec3 to_add) {force += to_add;};
  void add_force(float x, float y, float z) { force.x += x; force.y += y; force.z += z; };

  float get_volume();

  void set_mass(float new_mass);

  void set_infinite_mass();

};

class Point {

  bool model_matrix_deprecated = true;

  glm::mat4 m_model_matrix_buffer;

  glm::vec3 integration_position_buffer = {0,0,0};
  
  float m_pos_x = 0, m_pos_y = 0, m_pos_z = 0;
  
public:

  glm::mat4 get_model_matrix();
  
  void set_position(float x ,float y, float z);

  void buffer_integration_delta(glm::vec3 new_pos);
  void swap_integration_buffer();
  
  void change_position(float x ,float y, float z);
  void change_position(glm::vec3);
  
  glm::vec3 get_position();
  
  Point(float x, float y, float z);
  
  Physics_properties phys_props;

  unsigned int VAO_id;

  unsigned int VBO_vertices;

};
