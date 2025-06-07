#include "light.hh"

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

Light::Light(Mesh to_use)  : m_light_visualizer_mesh(to_use) {}

glm::vec3 Light::get_light_position() {

  return glm::vec3(m_light_matrix[3][0],
		   m_light_matrix[3][1],
		   m_light_matrix[3][2]);
  
}

glm::mat3 Light::get_light_rotation_matrix() {

  return glm::mat3(m_light_matrix);
  
}
