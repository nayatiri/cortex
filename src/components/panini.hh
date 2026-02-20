#pragma once

#include <glm/ext/matrix_float4x4.hpp>
namespace panini {

  glm::mat4 get_panini_projection_matrix(float fovy,
					 float aspect,
					 float zNear,
					 float zFar,
					 float d);
  
};
