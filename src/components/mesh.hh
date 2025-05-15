#pragma once

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

#include "../components/material.hh"

class Mesh {

public:

  Mesh(Material use_material);

  bool m_mesh_vbo_needs_refresh = true;
  
  void deserialize(char* file_path);
  
  std::vector<float> m_vertices_array;
  std::vector<float> m_tex_coords_array;
  std::vector<float> m_normals_array;
  std::vector<float> m_tangents_array;
  std::vector<float> m_binormals_array;

  GLuint m_mesh_vao;
  Material m_material;
  
  GLuint m_vertices_glid;
  GLuint m_tex_coords_glid;
  GLuint m_normals_glid;
  GLuint m_tangents_glid;
  GLuint m_binormals_glid;
  
};
