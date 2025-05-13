// logging.hh
#ifndef LOGGING_HH
#define LOGGING_HH

#include <glm/matrix.hpp>
#include <string>

void log_success(const std::string& message);
void log_debug(const std::string& message);
void log_debug_sub(const std::string& message);
void log_error(const std::string& message);
void log_mat_4(glm::mat4 mat);

#endif // LOGGING_HH
