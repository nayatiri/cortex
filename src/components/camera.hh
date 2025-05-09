#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stdlib
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdint>

class Camera {

  bool m_view_mat_initialized = false;
  glm::mat4 m_view_matrix;

  bool m_proj_mat_initialized = false;
  glm::mat4 m_projection_matrix;
  
};
