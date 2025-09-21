#pragma once

// my files
#include "../shaders/shaderclass.hh"

// stdlib
#include <glm/ext/vector_float4.hpp>

enum e_mat_type {
  E_PBR,
  E_PHONG
};

class Material {
public:

  Material(e_mat_type material_type);

  e_mat_type m_material_type = E_PHONG;
  bool transparent = false;
  bool full_pbr = false;
  
  //BSDF 
  const char* m_material_albedo_path = "";
  GLuint m_material_albedo_glid = 0;
  const char* m_material_metallic_roughness_path = "";
  GLuint m_material_metallic_roughness_glid = 0;
  const char* m_material_normal_path = "";
  GLuint m_material_normal_glid = 0;
  const char* m_material_displacement_path = "";
  GLuint m_material_displacement_glid = 0;

  glm::vec4 m_material_phong_base_color = glm::vec4(1.0f,0.0f,0.0f,1.0f);
  float metallic_factor = 0.0f;
  float roughness_factor = 0.0f;
  
  void material_pbr_tex_initialize();
  
};
