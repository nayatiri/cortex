#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shaders/shaderclass.hh"

// stdlib
#include <vector>

enum e_mesh_type {

  E_MESH,
  E_COL_BOX,
  E_SKYBOX

};

enum e_mesh_render_mode {

  E_WIREFRAME,
  E_FILLED
  
};

class AABB_Box {
private:
  glm::mat4 m_model_matrix_buffer;

public:

  glm::vec3 min_corner = {100000,100000,100000};
  glm::vec3 max_corner = {-100000,-100000,-100000};
  
  bool model_matrix_deprecated = true;
  float m_pos_x = 0, m_pos_y = 0, m_pos_z = 0;
  float m_rot_x = 0, m_rot_y = 0, m_rot_z = 0;
  glm::mat4 get_model_matrix();

  void set_position(float x ,float y, float z);
  void change_position(float x ,float y, float z);
  void set_rotation (float rx ,float ry, float rz);
  void change_rotation (float rx ,float ry, float rz);

  void exp_overwrite_model_matrix(glm::mat4 new_mat);
  
  AABB_Box(Shader new_hitbox_shader);

  Shader hitbox_shader;
  
  void deserialize(char* file_path);
  
  std::vector<float> m_vertices_array;

  GLuint m_mesh_vao;

  GLuint m_vertices_glid;

  e_mesh_type m_type = E_COL_BOX;
  e_mesh_render_mode m_render_mode = E_WIREFRAME;
  
};
