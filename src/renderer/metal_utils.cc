#include "renderer/metal_utils.h"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>

#include "platform/executable_path.h"
#include "platform/metal_ptr.h"
#include "renderer/metal_renderer.h"

using NS::TransferPtr;

namespace {
void PrintMetalError(const NS::Error& error) {
  std::cerr << error.localizedDescription()->utf8String() << "\n";
}
}  // namespace

PipelineStatePtr CreateRenderPipelineState(MTL::Device* device,
                                           const char* vertex_function_name,
                                           const char* fragment_function_name) {
  NS::Error* shader_library_error = nullptr;
  const std::string shader_library_path =
      (ExecutableDirectoryPath() / "shaders.metallib").string();
  NS::URL* shader_library_url =
      NS::URL::fileURLWithPath(ToNsString(shader_library_path.c_str()));

  MetalPtr<MTL::Library> shader_library = TransferPtr(
      device->newLibrary(shader_library_url, &shader_library_error));
  if (!shader_library && shader_library_error != nullptr) {
    PrintMetalError(*shader_library_error);
  }
  assert(
      shader_library &&
      "Shader library load failed: Could not create it from shaders.metallib.");

  MetalPtr<MTL::Function> vertex_main = TransferPtr(
      shader_library->newFunction(ToNsString(vertex_function_name)));
  assert(vertex_main &&
         "Vertex function lookup failed: Name not found in shaders.metallib.");

  MetalPtr<MTL::Function> fragment_main = TransferPtr(
      shader_library->newFunction(ToNsString(fragment_function_name)));
  assert(
      fragment_main &&
      "Fragment function lookup failed: Name not found in shaders.metallib.");

  MetalPtr<MTL::RenderPipelineDescriptor> pipeline_descriptor =
      CreateMetalObject<MTL::RenderPipelineDescriptor>();

  pipeline_descriptor->setVertexFunction(vertex_main.get());
  pipeline_descriptor->setFragmentFunction(fragment_main.get());
  pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(
      MetalRenderer::kColorPixelFormat);
  pipeline_descriptor->setDepthAttachmentPixelFormat(
      MetalRenderer::kDepthPixelFormat);

  NS::Error* pipeline_state_error = nullptr;
  PipelineStatePtr pipeline_state = TransferPtr(device->newRenderPipelineState(
      pipeline_descriptor.get(), &pipeline_state_error));
  if (!pipeline_state && pipeline_state_error != nullptr) {
    PrintMetalError(*pipeline_state_error);
  }
  assert(pipeline_state &&
         "Pipeline state creation failed: Metal rejected the descriptor.");

  return pipeline_state;
}

BufferPtr CreateBuffer(MTL::Device* device, const void* data,
                       std::size_t length) {
  // ResourceStorageModeManaged path is untested.
  const MTL::ResourceOptions storage_mode =
      device->hasUnifiedMemory() ? MTL::ResourceStorageModeShared
                                 : MTL::ResourceStorageModeManaged;
  MTL::Buffer* buffer = device->newBuffer(data, length, storage_mode);
  assert(buffer != nullptr &&
         "Buffer creation failed: Metal could not allocate the buffer.");

  return TransferPtr(buffer);
}

DepthStencilStatePtr CreateDepthStencilState(MTL::Device* device) {
  MetalPtr<MTL::DepthStencilDescriptor> depth_stencil_descriptor =
      CreateMetalObject<MTL::DepthStencilDescriptor>();

  assert(depth_stencil_descriptor &&
         "Depth stencil creation failed: Could not initialize descriptor.");

  depth_stencil_descriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
  depth_stencil_descriptor->setDepthWriteEnabled(true);

  MTL::DepthStencilState* depth_stencil_state =
      device->newDepthStencilState(depth_stencil_descriptor.get());
  assert(depth_stencil_state &&
         "Depth stencil creation failed: Could not initialize state.");

  return TransferPtr(depth_stencil_state);
}
