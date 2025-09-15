#pragma once

#include "point.hh"
#include <memory>

class Selectionstate {
public:

  std::shared_ptr<Point> selected_point = nullptr;

  bool launch_picker = false;
  
  int mouse_pos_x = 0, mouse_pos_y = 0;
  
};
