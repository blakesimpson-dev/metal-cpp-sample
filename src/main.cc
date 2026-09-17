#include <simd/simd.h>

#include <Metal/Metal.hpp>
#include <cassert>
#include <cstring>
#include <iostream>

constexpr const char* kShaderLibraryPath = "build/basic.metallib";

int main() {
  using NS::StringEncoding::UTF8StringEncoding;

  MTL::Device* device = MTL::CreateSystemDefaultDevice();
  MTL::CommandQueue* command_queue = device->newCommandQueue();
  assert(command_queue != nullptr && "Failed to create command queue.");

  std::cout << "Command Queue created.\n";
  const char* device_name = device->name()->utf8String();
  std::cout << "GPU Device Name: '" << device_name << "'.\n";

  NS::Error* shader_library_error = nullptr;
  NS::URL* shader_library_url = NS::URL::fileURLWithPath(
      NS::String::string(kShaderLibraryPath, UTF8StringEncoding));
  MTL::Library* shader_library =
      device->newLibrary(shader_library_url, &shader_library_error);
  assert(shader_library != nullptr && "Failed to create shader library.");

  MTL::Function* vertex_main = shader_library->newFunction(
      NS::String::string("VertexMain", UTF8StringEncoding));
  assert(vertex_main != nullptr && "Failed to create shader vertex function.");

  MTL::Function* fragment_main = shader_library->newFunction(
      NS::String::string("FragmentMain", UTF8StringEncoding));
  assert(fragment_main != nullptr &&
         "Failed to create shader fragment function.");

  MTL::RenderPipelineDescriptor* pipeline_descriptor =
      MTL::RenderPipelineDescriptor::alloc()->init();

  pipeline_descriptor->setVertexFunction(vertex_main);
  pipeline_descriptor->setFragmentFunction(fragment_main);
  pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

  NS::Error* pipeline_state_error = nullptr;
  MTL::RenderPipelineState* pipeline_state = device->newRenderPipelineState(
      pipeline_descriptor, &pipeline_state_error);
  assert(pipeline_state != nullptr &&
         "Failed to create render pipeline state.");

  // ...

  std::array positions{simd::float3{-0.675F, 0.675F, 0.0F},
                       simd::float3{0.0F, -0.675F, 0.0F},
                       simd::float3{+0.675F, 0.675F, 0.0F}};
  // constexpr size_t vertex_count = std::size(positions);

  std::array colors{simd::float3{1.0F, 0.0F, 0.0F},
                    simd::float3{0.0F, 1.0F, 0.0F},
                    simd::float3{0.0F, 0.0F, 1.0F}};

  MTL::ResourceOptions storage_mode = device->hasUnifiedMemory()
                                          ? MTL::ResourceStorageModeShared
                                          : MTL::ResourceStorageModeManaged;

  MTL::Buffer* positions_buffer =
      device->newBuffer(sizeof(positions), storage_mode);
  assert(positions_buffer != nullptr && "Failed to create positions buffer.");
  memcpy(positions_buffer->contents(), positions.data(), sizeof(positions));

  MTL::Buffer* colors_buffer = device->newBuffer(sizeof(colors), storage_mode);
  assert(colors_buffer != nullptr && "Failed to create colors buffer.");
  memcpy(colors_buffer->contents(), colors.data(), sizeof(colors));

  if (!device->hasUnifiedMemory()) {
    positions_buffer->didModifyRange(
        NS::Range::Make(0, positions_buffer->length()));
    colors_buffer->didModifyRange(NS::Range::Make(0, colors_buffer->length()));
  }

  // ...

  colors_buffer->release();
  positions_buffer->release();
  pipeline_state->release();
  pipeline_descriptor->release();
  fragment_main->release();
  vertex_main->release();
  shader_library->release();
  command_queue->release();
  device->release();
  return 0;
}
