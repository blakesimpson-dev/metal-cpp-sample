#pragma once

#include <Metal/Metal.hpp>
#include <chrono>

#include "platform/metal_ptr.h"
#include "renderer/camera.h"
#include "renderer/renderer.h"

// NOLINTNEXTLINE(readability-identifier-naming): Matches against metal-cpp.
namespace MTK {
class View;
}

class Scene;

class MetalRenderer : public Renderer {
 public:
  MetalRenderer(const MetalRenderer&) = delete;
  MetalRenderer& operator=(const MetalRenderer&) = delete;
  MetalRenderer(MTL::Device* device, MTK::View* view, Scene* scene);

  static constexpr MTL::PixelFormat kColorPixelFormat =
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB;
  static constexpr MTL::PixelFormat kDepthPixelFormat =
      MTL::PixelFormat::PixelFormatDepth32Float;

  void Draw() override;

 private:
  DevicePtr device_;
  CommandQueuePtr command_queue_;

  MTK::View* view_;
  Scene* scene_;
  Camera camera_;

  std::chrono::steady_clock::time_point last_frame_time_;
};
