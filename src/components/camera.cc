#include "camera.hh"
#include "logging.hh"

#include <glm/ext/matrix_clip_space.hpp>
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

  Logger::log_success("camera initialized");
  
}

void Camera::reset() {}

glm::mat4 Camera::get_view_matrix() { return camera_view_matrix; };

glm::mat4 Camera::get_projection_matrix() { return camera_projection_matrix; };

void Camera::set_view_matrix(glm::vec3 pos, glm::vec3 look_at, glm::vec3 up) {

  camera_view_matrix = glm::lookAt(pos,look_at,up);

};

void Camera::set_projection_matrix(float fov, float aspect_ratio,
                                   float clip_near, float clip_far) {

  camera_projection_matrix = glm::perspective(fov,aspect_ratio,clip_near,clip_far);
  
};

