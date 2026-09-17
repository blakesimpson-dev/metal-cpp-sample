#include <Metal/Metal.hpp>
#include <cassert>
#include <iostream>

constexpr const char* kMetallibPath = "build/basic.metallib";

int main() {
  using NS::StringEncoding::UTF8StringEncoding;

  MTL::Device* device = MTL::CreateSystemDefaultDevice();
  MTL::CommandQueue* command_queue = device->newCommandQueue();
  assert(command_queue != nullptr && "Failed to create command queue.");

  NS::Error* metallib_error = nullptr;
  NS::URL* metallib_url = NS::URL::fileURLWithPath(
      NS::String::string(kMetallibPath, UTF8StringEncoding));
  MTL::Library* shader_library =
      device->newLibrary(metallib_url, &metallib_error);
  assert(shader_library != nullptr && "Failed to create shader library.");

  std::cout << "Command Queue created.\n";
  const char* device_name = device->name()->utf8String();
  std::cout << "GPU Device Name: '" << device_name << "'.\n";

  command_queue->release();
  device->release();
  return 0;
}
