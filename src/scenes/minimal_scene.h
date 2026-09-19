#pragma once

#include <Metal/Metal.hpp>
#include <cstddef>

#include "scenes/scene.h"

class MinimalScene : public Scene {
 public:
  MinimalScene(const MinimalScene&) = delete;
  MinimalScene& operator=(const MinimalScene&) = delete;
  MinimalScene() = default;
  ~MinimalScene() override;
  void Load(MTL::Device* device) override;
  void Update(float delta) override;
  void Draw(MTL::RenderCommandEncoder* command_encoder) override;

 private:
  MTL::RenderPipelineState* pipeline_state_ = nullptr;
  MTL::Buffer* positions_buffer_ = nullptr;
  MTL::Buffer* colors_buffer_ = nullptr;
  size_t vertex_count_ = 0;
};
