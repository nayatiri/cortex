#include "light.hh"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Light::Light(std::shared_ptr<Mesh> to_use)  : m_light_visualizer_mesh(to_use) {}

glm::vec3 Light::get_light_position() {
  return m_light_position;
}

glm::mat4 Light::get_light_look_at() {
  return glm::lookAt(m_light_position,m_light_look_at_point, glm::vec3(0.0f,1.0f,0.0f));
}

void Light::set_light_look_at(float x,float y,float z) {
  m_light_look_at_point.x = x;
  m_light_look_at_point.y = y;
  m_light_look_at_point.z = z;
}

void Light::set_light_position(float x, float y, float z) {
  m_light_position.x = x;
  m_light_position.y = y;
  m_light_position.z = z;
}

glm::mat4 Light::get_light_rotation_matrix() {

  glm::vec3 forward = glm::normalize(m_light_look_at_point - m_light_position);

  glm::vec3 up(0,1,0);
  if (fabs(glm::dot(forward, up)) > 0.999f) {
    up = glm::vec3(0.0f, 0.0f, 1.0f);
  }
  
  glm::vec3 right = normalize(glm::cross(up,forward));

  glm::vec3 corrected_up = glm::cross(forward,right);
  
  return glm::mat4(right.x, corrected_up.x, forward.x, 0, 
		   right.y, corrected_up.y, forward.y, 0,
		   right.z, corrected_up.z, forward.z, 0,
		   0, 0, 0, 1);
  
}
