#include "entity.hh"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>

glm::mat4 Entity::get_model_matrix() {

  if(model_matrix_deprecated) {
    glm::mat4 rotation(1.0f);
    rotation = glm::rotate(rotation, m_rot_z,  glm::vec3(0, 0, 1)); // Z
    rotation = glm::rotate(rotation, m_rot_x, glm::vec3(1, 0, 0)); // X
    rotation = glm::rotate(rotation, m_rot_y,   glm::vec3(0, 1, 0)); // Y
    
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

void Entity::set_rotation(float x, float y, float z) {
  m_rot_x = x;
  m_rot_y = y;
  m_rot_z = z;
  model_matrix_deprecated = true;
}


void Entity::set_position(float x, float y, float z) {
  m_pos_x = x;
  m_pos_y = y;
  m_pos_z = z;
  model_matrix_deprecated = true;
}

void Entity::change_rotation(float x, float y, float z) {
  m_rot_x += x;
  m_rot_y += y;
  m_rot_z += z;
  model_matrix_deprecated = true;
}

void Entity::change_position(float x, float y, float z) {
  m_pos_x += x;
  m_pos_y += y;
  m_pos_z += z;
  model_matrix_deprecated = true;
}

void Entity::change_position(glm::vec3 add_vec) {
  m_pos_x += add_vec.x;
  m_pos_y += add_vec.y;
  m_pos_z += add_vec.z;
  model_matrix_deprecated = true;
}


bool Entity::is_initialized() {
  if(m_mesh.size()>0)
    return true;
  else
    return false;
}

glm::vec3 Entity::get_position() {

  return glm::vec3(m_pos_x,m_pos_y,m_pos_z);
  
}
