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
  e_light_type m_light_type;

  float m_strength;
  uint64_t m_color;

  float light_width = 10.0f;
  
  Mesh m_light_visualizer_mesh;
  
  glm::mat4 m_light_matrix = glm::mat4(1.0f);

  glm::vec3 get_light_position();
  glm::mat3 get_light_rotation_matrix();
  
  Light(Mesh to_use);
  
};
