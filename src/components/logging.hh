#pragma once

#include <glm/matrix.hpp>
#include <string>
namespace Logger {
  
  enum class Level { Debug, Success, Warning, Error, Critical};

  inline const char* to_string(Level lvl);
  
  template <class... Args>
  void log(Level lvl, Args&&... args);
  
  void log_success(const std::string& message);
  void log_debug(const std::string& message);
  void log_debug_sub(const std::string& message);
  void log_error(const std::string& message);
  void log_mat_4(glm::mat4 mat);
  
};
