#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stdlib
#include <memory>
#include <vector>

#include "../components/material.hh"
#include "AABB.hh"


class Mesh {
private:
  glm::mat4 m_model_matrix_buffer = glm::mat4(1.0f);

public:
  std::shared_ptr<AABB_Box> AABB_visualizer = nullptr;
  
  bool model_matrix_deprecated = true;
  float m_pos_x = 0, m_pos_y = 0, m_pos_z = 0;
  float m_rot_x = 0, m_rot_y = 0, m_rot_z = 0;
  float m_sca_x = 0, m_sca_y = 0, m_sca_z = 0;
  glm::mat4 get_model_matrix();

  void set_position(float x ,float y, float z);
  void change_position(float x ,float y, float z);

  void set_rotation (float rx ,float ry, float rz);
  void change_rotation (float rx ,float ry, float rz);

  void exp_overwrite_model_matrix(glm::mat4 new_mat);

  Mesh() = default;
  Mesh(std::shared_ptr<Material> set_material);
  ~Mesh() = default;

  bool m_mesh_vbo_needs_refresh = true;
  
  std::vector<float> m_vertices_array;
  std::vector<float> m_tex_coords_array;
  std::vector<float> m_normals_array;
  std::vector<float> m_tangents_array;
  std::vector<float> m_binormals_array;

  GLuint m_mesh_vao = 0;
  std::shared_ptr<Material> m_material = nullptr;

  GLuint m_vertices_glid = 0;
  GLuint m_tex_coords_glid = 0;
  GLuint m_normals_glid = 0;
  GLuint m_tangents_glid = 0;
  GLuint m_binormals_glid = 0;

  e_mesh_type m_type = E_MESH;
  e_mesh_render_mode m_render_mode = E_FILLED;
  
};
