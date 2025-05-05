#include "material.hh"

#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stdlib
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

//material constructor
Material::Material(e_mat_type material_type, Shader use_shader) : m_shader(use_shader) {
  
  m_material_type = material_type;
  
}

