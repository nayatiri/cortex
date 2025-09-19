#include "camera.hh"
#include "logging.hh"

#include <glm/ext/matrix_float4x4.hpp>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stdlib
#include <string>

Camera::Camera() {

  log_success("camera initialized");
  
}

void Camera::reset() {}
