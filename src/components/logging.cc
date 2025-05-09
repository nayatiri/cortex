#include "logging.hh"
#include <iostream>

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
