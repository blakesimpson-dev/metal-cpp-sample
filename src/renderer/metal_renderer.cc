#include "renderer/metal_renderer.h"

#include <CoreFoundation/CFCGTypes.h>

#include <cassert>
#include <chrono>
#include <cstddef>

#include <MetalKit/MetalKit.hpp>

#include "platform/fatal_error.h"
#include "platform/metal_ptr.h"
#include "renderer/frames_in_flight.h"
#include "scenes/scene.h"

MetalRenderer::MetalRenderer(MTL::Device* device, MTK::View* view, Scene* scene)
    : device_(NS::RetainPtr(device)),
      command_queue_(NS::TransferPtr(device->newCommandQueue())),
      view_(view),
      scene_(scene) {
  Check(command_queue_.get() != nullptr,
        "Command queue creation failed: Could not initialize.");
  Check(device_->supportsTextureSampleCount(MetalRenderer::SampleCount()),
        "Sample count check failed: Device does not support this MSAA level.");

  scene_->Load(device_.get());
  scene_->ConfigureCamera(camera_);
  const CGSize size = view_->drawableSize();
  // assert over Check here is intentional
  assert(size.height > 0.0 && "Camera setup failed: View has no height.");

  camera_.SetAspectRatio(static_cast<float>(size.width / size.height));
  view_->setClearColor(scene_->SceneClearColor());
  last_frame_time_ = std::chrono::steady_clock::now();
}

MetalRenderer::~MetalRenderer() {
  for (std::size_t i = 0; i < kMaxFramesInFlight; i++) {
    frame_semaphore_.acquire();
  }
}

void MetalRenderer::Draw() {
  const std::chrono::steady_clock::time_point now =
      std::chrono::steady_clock::now();
  const float delta =
      std::chrono::duration<float>(now - last_frame_time_).count();

  last_frame_time_ = now;
  scene_->Update(delta);

  AutoreleasePoolPtr pool = CreateMetalObject<NS::AutoreleasePool>();

  frame_semaphore_.acquire();

  MTL::RenderPassDescriptor* render_pass = view_->currentRenderPassDescriptor();
  CA::MetalDrawable* drawable = view_->currentDrawable();
  if (render_pass == nullptr || drawable == nullptr) {
    frame_semaphore_.release();
    return;
  }

  frame_index_ = (frame_index_ + 1) % kMaxFramesInFlight;

  MTL::CommandBuffer* command_buffer = command_queue_->commandBuffer();
  command_buffer->addCompletedHandler(
      [this](MTL::CommandBuffer* /*buffer*/) { frame_semaphore_.release(); });

  MTL::RenderCommandEncoder* command_encoder =
      command_buffer->renderCommandEncoder(render_pass);

  scene_->Draw(command_encoder, camera_, frame_index_);

  command_encoder->endEncoding();
  command_buffer->presentDrawable(drawable);
  command_buffer->commit();
}
