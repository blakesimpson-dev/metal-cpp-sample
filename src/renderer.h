#pragma once

#include <Metal/Metal.hpp>

namespace MTK {  // NOLINT(readability-identifier-naming)
class View;
}

class Renderer {
 public:
  static constexpr MTL::PixelFormat kColorPixelFormat =
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB;

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  explicit Renderer(MTL::Device* device);
  ~Renderer();
  void Draw(MTK::View* view);

 private:
  MTL::Device* device_;
  MTL::CommandQueue* command_queue_;
  MTL::RenderPipelineState* pipeline_state_;
  MTL::Buffer* positions_buffer_;
  MTL::Buffer* colors_buffer_;
  size_t vertex_count_;
};
