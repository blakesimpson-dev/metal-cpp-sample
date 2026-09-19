#pragma once

#include <Metal/Metal.hpp>
#include <cstddef>

#include "renderer.h"

namespace MTK {  // NOLINT(readability-identifier-naming)
class View;
}

class Scene;

class MetalRenderer : public Renderer {
 public:
  static constexpr MTL::PixelFormat kColorPixelFormat =
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB;

  MetalRenderer(const MetalRenderer&) = delete;
  MetalRenderer& operator=(const MetalRenderer&) = delete;
  MetalRenderer(MTL::Device* device, MTK::View* view, Scene* scene);
  ~MetalRenderer() override;
  void Draw() override;

 private:
  MTL::Device* device_;
  MTL::CommandQueue* command_queue_;
  MTK::View* view_;
  Scene* scene_;
  size_t vertex_count_;
};
