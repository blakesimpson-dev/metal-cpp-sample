#include <Metal/Metal.hpp>
#include <cassert>
#include <iostream>

int main() {
  MTL::Device* device = MTL::CreateSystemDefaultDevice();
  MTL::CommandQueue* command_queue = device->newCommandQueue();
  assert(command_queue != nullptr && "Failed to create command queue.");

  std::cout << "Command Queue created.\n";
  const char* device_name = device->name()->utf8String();
  std::cout << "GPU Device Name: '" << device_name << "'.\n";

  command_queue->release();
  device->release();
  return 0;
}
