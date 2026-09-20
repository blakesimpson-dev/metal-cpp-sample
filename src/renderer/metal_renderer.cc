#include "renderer/metal_renderer.h"

#include <simd/simd.h>

#include <MetalKit/MetalKit.hpp>
#include <cassert>
#include <chrono>
#include <cstring>

#include "scenes/scene.h"

MetalRenderer::MetalRenderer(MTL::Device* device, MTK::View* view, Scene* scene)
    : device_(device->retain()),
      command_queue_(device->newCommandQueue()),
      view_(view),
      scene_(scene) {
  assert(command_queue_ != nullptr && "Failed to create command queue.");
  scene_->Load(device_);
  last_frame_time_ = std::chrono::steady_clock::now();
}

MetalRenderer::~MetalRenderer() {
  command_queue_->release();
  device_->release();
}

void MetalRenderer::Draw() {
  const std::chrono::steady_clock::time_point now =
      std::chrono::steady_clock::now();
  const float delta =
      std::chrono::duration<float>(now - last_frame_time_).count();
  last_frame_time_ = now;
  scene_->Update(delta);

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
