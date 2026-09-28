#include "platform/executable_path.h"

#include <cstdint>
#include <filesystem>
#include <vector>

#include <mach-o/dyld.h>

std::filesystem::path ExecutableDirectoryPath() {
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::vector<char> buffer(size);
  _NSGetExecutablePath(buffer.data(), &size);

  return std::filesystem::canonical(buffer.data()).parent_path();
}
