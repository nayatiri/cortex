#pragma once

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

class Mesh {

public:

  void deserialize(char* file_path);
  
  std::vector<float> m_vertices_array;
  std::vector<float> m_tex_coords_array;
  std::vector<float> m_normals_array;
  std::vector<float> m_tangents_array;
  std::vector<float> m_bitangents_array;

  GLuint m_mesh_vao;
  
  GLuint m_vertices_glid;
  GLuint m_tex_coords_glid;
  GLuint m_normals_glid;
  GLuint m_tangents_glid;
  GLuint m_bitangents_glid;
  
};
