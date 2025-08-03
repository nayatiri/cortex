#pragma once

#include <memory>
#include <vector>
#include "textbox.hh"
#include "blankbox.hh"
#include "../shaders/shaderclass.hh"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Overlay {
public:

  std::shared_ptr<std::vector<textbox>> text_elements;
  std::shared_ptr<std::vector<blankbox>> structure_elements;

  std::unique_ptr<GLuint> text_vbo;
  std::unique_ptr<Shader> text_shader;

};
