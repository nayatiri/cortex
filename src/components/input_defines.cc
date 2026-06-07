#include "input_defines.hh"
#include "input.hh"
#include "logging.hh"
#include <GLFW/glfw3.h>

// Area to link actual implementation to key
void InputDefinitions::register_input_definitions(Input_Manager& im) {

  Logger::log_debug("Trying to register input links to callbacks.");

  im.register_falling_edge(InputDefinitions::test_function, GLFW_KEY_A);
  
}
void InputDefinitions::test_function() {

  Logger::log_success("test function called!");
  
}
