#pragma once


#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
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
#include <iostream>

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

  glm::mat4 m_light_matrix;
  
};
