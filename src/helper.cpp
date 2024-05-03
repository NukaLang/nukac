#include <format>
#include <iostream>

#define RED_ERROR "\033[38;2;166;17;17m"
#define RESET "\033[0m"

#include "helper.hpp"

namespace nukac::helper {

  void exceptionHandler(std::string msg) {
    std::cout << std::format("{}error:{} {}\n\t...no useful hints ;-;\n", RED_ERROR, msg, RESET);
  }

}
