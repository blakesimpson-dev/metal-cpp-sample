#include "platform/fatal_error.h"

#include <cstdlib>
#include <iostream>
#include <string_view>

void FatalError(std::string_view message) {
  std::cerr << "Error: " << message << "\n";
  std::exit(EXIT_FAILURE);
}
