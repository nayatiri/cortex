#include "mesh.hh"

Mesh::Mesh(std::shared_ptr<Material> use_material) : m_material(use_material) {}

glm::mat4 Mesh::get_model_matrix() {

  if (model_matrix_deprecated) {

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(m_pos_x, m_pos_y, m_pos_z));

    model = glm::rotate(model, glm::radians(m_rot_x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(m_rot_y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(m_rot_z), glm::vec3(0, 0, 1));

    model = glm::scale(model, glm::vec3(m_sca_x, m_sca_y, m_sca_z));

    m_model_matrix_buffer = model;
    model_matrix_deprecated = false;
    return model;
  }

  return m_model_matrix_buffer;
}

void Mesh::set_rotation(float x, float y, float z) {
  m_rot_x = x;
  m_rot_y = y;
  m_rot_z = z;
  model_matrix_deprecated = true;
  phys_box_needs_recalculation = true;
}

void Mesh::set_position(float x, float y, float z) {
  m_pos_x = x;
  m_pos_y = y;
  m_pos_z = z;
  model_matrix_deprecated = true;
  phys_box_needs_recalculation = true;
}

void Mesh::change_rotation(float x, float y, float z) {
  m_rot_x += x;
  m_rot_y += y;
  m_rot_z += z;
  model_matrix_deprecated = true;
  phys_box_needs_recalculation = true;
}

void Mesh::change_position(float x, float y, float z) {
  m_pos_x += x;
  m_pos_y += y;
  m_pos_z += z;
  model_matrix_deprecated = true;
  phys_box_needs_recalculation = true;
}

void Mesh::exp_overwrite_model_matrix(glm::mat4 new_mat) {
  m_model_matrix_buffer = new_mat;
};
