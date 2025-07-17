#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "animation.hh"

class Camera {
public:
  float m_camera_base_speed = 1.0f;
  glm::vec3 m_cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
  glm::vec3 m_cameraLookAt = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 m_direction = {0.0f, 0.0f, 0.0f};
  
  bool m_view_mat_initialized = false;
  glm::mat4 m_view_matrix;

  bool m_proj_mat_initialized = false;
  glm::mat4 m_projection_matrix;

  std::vector<animation*>* m_animation_table = nullptr;
  
  Camera();
  void reset();
  
};
