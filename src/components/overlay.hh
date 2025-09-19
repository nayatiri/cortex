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

  std::shared_ptr<std::vector<textbox>> text_elements = nullptr;
  std::shared_ptr<std::vector<blankbox>> structure_elements = nullptr;

  std::unique_ptr<GLuint> text_vbo = nullptr;
  std::unique_ptr<Shader> text_shader = nullptr;

};
