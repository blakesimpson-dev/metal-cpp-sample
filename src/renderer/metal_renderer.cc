#include "renderer/metal_renderer.h"

#include <CoreFoundation/CFCGTypes.h>

#include <MetalKit/MetalKit.hpp>
#include <cassert>
#include <chrono>

#include "platform/metal_ptr.h"
#include "scenes/scene.h"

MetalRenderer::MetalRenderer(MTL::Device* device, MTK::View* view, Scene* scene)
    : device_(NS::RetainPtr(device)),
      command_queue_(NS::TransferPtr(device->newCommandQueue())),
      view_(view),
      scene_(scene) {
  assert(command_queue_ &&
         "Command queue creation failed: Could not initialize.");
  assert(device_->supportsTextureSampleCount(MetalRenderer::SampleCount()) &&
         "Sample count check failed: Device does not support this MSAA level.");

  scene_->Load(device_.get());
  scene_->ConfigureCamera(camera_);
  const CGSize size = view_->drawableSize();
  assert(size.height > 0.0 && "Camera setup failed: View has no height.");

  camera_.SetAspectRatio(static_cast<float>(size.width / size.height));
  view_->setClearColor(scene_->SceneClearColor());
  last_frame_time_ = std::chrono::steady_clock::now();
}

void MetalRenderer::Draw() {
  const std::chrono::steady_clock::time_point now =
      std::chrono::steady_clock::now();
  const float delta =
      std::chrono::duration<float>(now - last_frame_time_).count();

  last_frame_time_ = now;
  scene_->Update(delta);

  AutoreleasePoolPtr pool = CreateMetalObject<NS::AutoreleasePool>();

  MTL::CommandBuffer* command_buffer = command_queue_->commandBuffer();
  MTL::RenderPassDescriptor* render_pass = view_->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* command_encoder =
      command_buffer->renderCommandEncoder(render_pass);

  scene_->Draw(command_encoder, camera_);

  command_encoder->endEncoding();
  command_buffer->presentDrawable(view_->currentDrawable());
  command_buffer->commit();
}
