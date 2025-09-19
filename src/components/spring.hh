#pragma once

#include "point.hh"
#include <memory>
class Spring {
public:

  Spring(std::shared_ptr<Point> from, std::shared_ptr<Point> to, float strength);
  
  std::shared_ptr<Point> link_A = nullptr;
  std::shared_ptr<Point> link_B = nullptr;

  float strength = 50.0f;

  unsigned int VAO_id = 0;

  unsigned int VBO_vertices = 0;
  
};
