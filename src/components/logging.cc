#include "logging.hh"
#include <iostream>
#include <glm/matrix.hpp>

void log_success(const std::string& input) {
  std::cout << "\033[1;32m[S] " << input << "\033[0m" << std::endl;
}

void log_debug(const std::string& input) {
  std::cout << "\033[1;37m[D] " << input << "\033[0m" << std::endl;
}

void log_debug_sub(const std::string& input) {
  std::cout << "\033[1;90m[D] - " << input << "\033[0m" << std::endl;
}

void log_error(const std::string& input) {
  std::cout << "\033[1;31m[E] " << input << "\033[0m" << std::endl;
}

void log_mat_4(glm::mat4 mat) {
    for (int i = 0; i < 4; ++i) {
        std::cout << "| ";
        for (int j = 0; j < 4; ++j) {
            std::cout << mat[i][j] << " ";
        }
        std::cout << "|" << std::endl;
    }
}
