#pragma once

#include "point.hh"
#include <memory>
class Spring {
public:

  Spring(std::shared_ptr<Point> from, std::shared_ptr<Point> to, float strength);
  
  std::shared_ptr<Point> link_A;
  std::shared_ptr<Point> link_B;

  float strength;

  unsigned int VAO_id;

  unsigned int VBO_vertices;
  
};
