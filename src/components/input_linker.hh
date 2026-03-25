#pragma once

#include <functional>

class Input_Linker {

  // link a function callback to a GLFW int KeySym
  void register_falling_edge(std::function<void(int)> fun, int key_sym); 
  void register_rising_edge(std::function<void> fun, int key_sym); 
  
};
