#include "mesh.hh"

void Mesh::deserialize(char* file_path) {

  return;
}

Mesh::Mesh(Material use_material) : m_material(use_material) {}


glm::mat4 Mesh::get_model_matrix() {

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

void Mesh::set_rotation(float x, float y, float z) {
  m_rot_x = x;
  m_rot_y = y;
  m_rot_z = z;
  model_matrix_deprecated = true;
}


void Mesh::set_position(float x, float y, float z) {
  m_pos_x = x;
  m_pos_y = y;
  m_pos_z = z;
  model_matrix_deprecated = true;
}

void Mesh::change_rotation(float x, float y, float z) {
  m_rot_x += x;
  m_rot_y += y;
  m_rot_z += z;
  model_matrix_deprecated = true;
}

void Mesh::change_position(float x, float y, float z) {
  m_pos_x += x;
  m_pos_y += y;
  m_pos_z += z;
  model_matrix_deprecated = true;
}

void Mesh::exp_overwrite_model_matrix(glm::mat4 new_mat) {
  m_model_matrix_buffer = new_mat;
};
