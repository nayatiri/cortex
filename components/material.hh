#pragma once

#include <iostream>

// my files
#include "../shaders/shader.hh"

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

// assimp
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstdint>

enum e_mat_type {
  E_PBR,
  E_PHONG,
  E_PHONG_TEX,
  E_FLAT,
  E_FLAT_TEX
};

struct s_material_pbr {

  Shader m_shader_linked_shader_class;
  
};
struct s_material_pbr_tex {
  
  Shader m_shader_linked_shader_class;
  
};
struct s_material_phong {

  Shader m_shader_linked_shader_class;
  
};
struct s_material_phong_tex {
  
  Shader m_shader_linked_shader_class;

};
struct s_material_face {

  Shader m_shader_linked_shader_class;
  
};
struct s_material_face_tex {

  Shader m_shader_linked_shader_class;
  
};
