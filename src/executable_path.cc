#include "executable_path.h"

#include <mach-o/dyld.h>

std::filesystem::path ExecutableDir() {
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::vector<char> buffer(size);
  _NSGetExecutablePath(buffer.data(), &size);

  return std::filesystem::canonical(buffer.data()).parent_path();
}
