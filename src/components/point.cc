#include "point.hh"
#include "force_generator.hh"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <memory>

Point::Point(float x, float y, float z) {
  m_pos_x = x;
  m_pos_y = y;
  m_pos_z = z;
};

glm::mat4 Point::get_model_matrix() {

  if(model_matrix_deprecated) {
    glm::mat4 rotation(1.0f);
    
    rotation[3] = glm::vec4(m_pos_x, m_pos_y, m_pos_z,1.0);
    rotation[0][3] = 0;
    rotation[1][3] = 0;
    rotation[2][3] = 0;

    model_matrix_deprecated = false;

    m_model_matrix_buffer = rotation;
    return rotation;
  }

  return m_model_matrix_buffer;
}

void Point::change_position(float x, float y, float z) {
  m_pos_x += x;
  m_pos_y += y;
  m_pos_z += z;
  model_matrix_deprecated = true;
}

void Point::change_position(glm::vec3 add_vec) {
  m_pos_x += add_vec.x;
  m_pos_y += add_vec.y;
  m_pos_z += add_vec.z;
  model_matrix_deprecated = true;
}

void Point::set_position(float x, float y, float z) {
  m_pos_x = x;
  m_pos_y = y;
  m_pos_z = z;
  model_matrix_deprecated = true;
}

glm::vec3 Point::get_position() { return glm::vec3(m_pos_x, m_pos_y, m_pos_z); }

