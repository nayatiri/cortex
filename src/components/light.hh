#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stdlib
#include <cstdint>

#include "mesh.hh"

enum e_light_type {

  E_POINT_LIGHT,
  E_SPOT_LIGHT,
  E_AMBIENT
  
};

class Light {
public:
  e_light_type m_light_type = E_POINT_LIGHT;

  float m_strength = 100.0f;
  uint64_t m_color = 0xFFFFFF;

  float light_width = 10.0f;
  
  std::shared_ptr<Mesh> m_light_visualizer_mesh = nullptr;

  glm::vec3 m_light_position = glm::vec3(0.0f);
  glm::vec3 m_light_look_at_point = glm::vec3(0.0f);

  glm::mat4 get_light_look_at();
  glm::vec3 get_light_position();
  glm::mat4 get_light_rotation_matrix();

  void set_light_position(float x,float y,float z);
  void set_light_look_at(float x,float y,float z);
  
  Light(std::shared_ptr<Mesh> to_use);
  
};
