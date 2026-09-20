#include "renderer/metal_utils.h"

#include <Metal/Metal.hpp>
#include <cassert>
#include <cstddef>
#include <string>

#include "platform/executable_path.h"
#include "renderer/metal_renderer.h"

MTL::RenderPipelineState* CreateRenderPipelineState(
    MTL::Device* device, const char* vertex_function_name,
    const char* fragment_function_name) {
  NS::Error* shader_library_error = nullptr;
  const std::string shader_library_path =
      (ExecutableDirectoryPath() / "shaders.metallib").string();
  NS::URL* shader_library_url = NS::URL::fileURLWithPath(NS::String::string(
      shader_library_path.c_str(), NS::StringEncoding::UTF8StringEncoding));

  MTL::Library* shader_library =
      device->newLibrary(shader_library_url, &shader_library_error);
  assert(
      shader_library != nullptr &&
      "Shader library load failed: Could not create it from shaders.metallib.");

  MTL::Function* vertex_main = shader_library->newFunction(NS::String::string(
      vertex_function_name, NS::StringEncoding::UTF8StringEncoding));
  assert(vertex_main != nullptr &&
         "Vertex function lookup failed: Name not found in shaders.metallib.");

  MTL::Function* fragment_main = shader_library->newFunction(NS::String::string(
      fragment_function_name, NS::StringEncoding::UTF8StringEncoding));
  assert(
      fragment_main != nullptr &&
      "Fragment function lookup failed: Name not found in shaders.metallib.");

  MTL::RenderPipelineDescriptor* pipeline_descriptor =
      MTL::RenderPipelineDescriptor::alloc()->init();

  pipeline_descriptor->setVertexFunction(vertex_main);
  pipeline_descriptor->setFragmentFunction(fragment_main);
  pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(
      MetalRenderer::kColorPixelFormat);
  pipeline_descriptor->setDepthAttachmentPixelFormat(
      MetalRenderer::kDepthPixelFormat);

  NS::Error* pipeline_state_error = nullptr;
  MTL::RenderPipelineState* pipeline_state = device->newRenderPipelineState(
      pipeline_descriptor, &pipeline_state_error);
  assert(pipeline_state != nullptr &&
         "Pipeline state creation failed: Metal rejected the descriptor.");

  pipeline_descriptor->release();
  fragment_main->release();
  vertex_main->release();
  shader_library->release();

  return pipeline_state;
}

MTL::Buffer* CreateBuffer(MTL::Device* device, const void* data,
                          std::size_t length) {
  // NOTE: ResourceStorageModeManaged path is untested (I don't have access to
  // a Mac with dedicated graphics..!)
  const MTL::ResourceOptions storage_mode =
      device->hasUnifiedMemory() ? MTL::ResourceStorageModeShared
                                 : MTL::ResourceStorageModeManaged;
  MTL::Buffer* buffer = device->newBuffer(data, length, storage_mode);
  assert(buffer != nullptr &&
         "Buffer creation failed: Metal could not allocate the buffer.");

  return buffer;
}

MTL::DepthStencilState* CreateDepthStencilState(MTL::Device* device) {
  MTL::DepthStencilDescriptor* depth_stencil_descriptor =
      MTL::DepthStencilDescriptor::alloc()->init();

  depth_stencil_descriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
  depth_stencil_descriptor->setDepthWriteEnabled(true);

  MTL::DepthStencilState* depth_stencil_state =
      device->newDepthStencilState(depth_stencil_descriptor);

  depth_stencil_descriptor->release();
  return depth_stencil_state;
}
