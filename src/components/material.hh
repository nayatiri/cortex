#pragma once

#include <iostream>

// my files
#include "../shaders/shaderclass.hh"

// stdlib
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdint>

#include <cstdint>

enum e_mat_type {
  E_PBR,
  E_PBR_TEX,
  E_PHONG,
  E_PHONG_TEX,
  E_FACE,
  E_FACE_TEX
};

class Material {
public:
  //horrible impl, but like impossible to do with polymorphism if i want a material to be a member var in the mesh

  e_mat_type m_material_type;

  Material(e_mat_type material_type, Shader use_shader);

  Shader m_shader;
  int bound_texture_id = -1;
  
  //pbr with textures
  const char* m_material_pbr_tex_albedo_path;
  const char* m_material_pbr_tex_metallic_path;
  const char* m_material_pbr_tex_roughness_path;
  const char* m_material_pbr_tex_normal_path;
  const char* m_material_pbr_tex_displacement_path;

  void material_pbr_tex_initialize();

  //phong with textures
  const char* m_material_phong_tex_path;
  float m_material_phong_tex_diffuse_pow;
  float m_material_phong_tex_specular_pow;

  //phong without textures
  unsigned int m_material_phong_base_color;
  float m_material_phong_diffuse_pow;
  float m_material_phong_specular_pow;

  //flat shading

  unsigned int m_material_flat_base_color;
  
};
