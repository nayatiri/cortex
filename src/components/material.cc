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
Material::Material(e_mat_type material_type) {

  m_material_type = material_type;

  switch(m_material_type) {
    
  case E_FACE: {

    Shader mat_shader("shader/flat.vert", "shader/flat.vert");
    
    m_shader = mat_shader;
    
    break;
  }
  case E_FACE_TEX: {break;}
  case E_PBR: {break;}
  case E_PBR_TEX: {break;}
  case E_PHONG: {break;}
  case E_PHONG_TEX: {break;}

  }
  
}

