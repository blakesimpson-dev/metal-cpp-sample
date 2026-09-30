#ifndef METAL_CPP_SAMPLE_PLATFORM_FATAL_ERROR_H_
#define METAL_CPP_SAMPLE_PLATFORM_FATAL_ERROR_H_

#include <string_view>

[[noreturn]] void FatalError(std::string_view message);

inline void Check(bool condition, std::string_view message) {
  if (!condition) {
    FatalError(message);
  }
}

#endif  // METAL_CPP_SAMPLE_PLATFORM_FATAL_ERROR_H_
