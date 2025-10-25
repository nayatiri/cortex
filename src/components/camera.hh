#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
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

  glm::mat4 camera_view_matrix = glm::mat4(1.0f);
  glm::mat4 camera_projection_matrix = glm::mat4(1.0f);
  
  float fov = 90.0f;
  
  std::vector<animation*>* m_animation_table = nullptr;
  
  Camera();
  void reset();

  void set_view_matrix(glm::vec3 pos, glm::vec3 look_at, glm::vec3 up);
  void set_projection_matrix(float fov, float aspect_ratio, float clip_near, float clip_far);
  
  glm::mat4 get_view_matrix();
  glm::mat4 get_projection_matrix();
  
};
