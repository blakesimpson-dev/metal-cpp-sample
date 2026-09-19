#include "renderer/metal_renderer.h"

#include <simd/simd.h>

#include <MetalKit/MetalKit.hpp>
#include <cassert>
#include <cstring>

#include "scenes/scene.h"

namespace {
constexpr const char* kVertexShaderFunctionName = "VertexMain";
constexpr const char* kFragmentShaderFunctionName = "FragmentMain";
constexpr NS::UInteger kPositionsBufferIndex = 0;
constexpr NS::UInteger kColorsBufferIndex = 1;
}  // namespace

MetalRenderer::MetalRenderer(MTL::Device* device, MTK::View* view, Scene* scene)
    : device_(device->retain()), view_(view), scene_(scene) {
  command_queue_ = device_->newCommandQueue();
  assert(command_queue_ != nullptr && "Failed to create command queue.");

  scene_->Load(device_);
}

MetalRenderer::~MetalRenderer() {
  command_queue_->release();
  device_->release();
}

void MetalRenderer::Draw() {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer* command_buffer = command_queue_->commandBuffer();
  MTL::RenderPassDescriptor* render_pass = view_->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* command_encoder =
      command_buffer->renderCommandEncoder(render_pass);

  scene_->Draw(command_encoder);

  command_encoder->endEncoding();
  command_buffer->presentDrawable(view_->currentDrawable());
  command_buffer->commit();

  pool->release();
}
