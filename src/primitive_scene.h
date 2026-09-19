#pragma once

#include <Metal/Metal.hpp>
#include <cstddef>

#include "scene.h"

class PrimitiveScene : public Scene {
 public:
  PrimitiveScene(const PrimitiveScene&) = delete;
  PrimitiveScene& operator=(const PrimitiveScene&) = delete;
  PrimitiveScene() = default;
  ~PrimitiveScene() override;
  void Load(MTL::Device* device) override;
  void Update(float delta) override;
  void Draw(MTL::RenderCommandEncoder* command_encoder) override;

 private:
  MTL::RenderPipelineState* pipeline_state_ = nullptr;
  MTL::Buffer* positions_buffer_ = nullptr;
  MTL::Buffer* colors_buffer_ = nullptr;
  size_t vertex_count_ = 0;
};
