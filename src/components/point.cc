#include "point.hh"
#include "force_generator.hh"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <iostream>
#include <memory>

Point::Point(float x, float y, float z) {
  m_pos_x = x;
  m_pos_y = y;
  m_pos_z = z;
};

float Point::get_distance_to_other_point(const std::shared_ptr<Point>& q) {

  return glm::length(get_position() - q->get_position());
  
}

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

void Point::log_position() {

  std::cout << "Point position x,y,z: " << m_pos_x << " , " << m_pos_y << " , " << m_pos_z << std::endl;
  
};

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

float Physics_properties::get_volume() { return 0.75 * 3.141 * pow(radius, 3); }

void Physics_properties::set_mass(float new_mass) {
  inverse_mass = 1 / new_mass;
}

void Physics_properties::set_infinite_mass() { inverse_mass = 0.0f; }

void Point::buffer_integration_delta(glm::vec3 new_pos) {
  integration_position_buffer = new_pos;
};

void Point::swap_integration_buffer() {

  m_pos_x += integration_position_buffer.x;
  m_pos_y += integration_position_buffer.y;
  m_pos_z += integration_position_buffer.z;
  
}
