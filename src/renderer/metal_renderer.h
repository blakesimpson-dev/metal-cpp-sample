#pragma once

#include <Metal/Metal.hpp>
#include <chrono>

#include "platform/metal_ptr.h"
#include "renderer/camera.h"
#include "renderer/renderer.h"

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

  static void SetMsaaEnabled(bool enabled) { msaa_enabled_ = enabled; }

  [[nodiscard]]
  static NS::UInteger SampleCount() {
    return msaa_enabled_ ? kMsaaMaxSampleCount : kMsaaMinSampleCount;
  }

 private:
  static inline bool msaa_enabled_ = true;
  static constexpr NS::UInteger kMsaaMinSampleCount = 1;
  static constexpr NS::UInteger kMsaaMaxSampleCount = 4;

  DevicePtr device_;
  CommandQueuePtr command_queue_;

  MTK::View* view_;
  Scene* scene_;
  Camera camera_;

  std::chrono::steady_clock::time_point last_frame_time_;
};
