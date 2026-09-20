#pragma once

#include <Metal/Metal.hpp>

#include "scenes/gltf_model.h"
#include "scenes/scene.h"

class GltfScene : public Scene {
 public:
  GltfScene(const GltfScene&) = delete;
  GltfScene& operator=(const GltfScene&) = delete;
  GltfScene() = default;
  ~GltfScene() override;
  void Load(MTL::Device* device) override;
  void Update(float delta) override;
  void Draw(MTL::RenderCommandEncoder* command_encoder) override;

 private:
  MTL::RenderPipelineState* pipeline_state_ = nullptr;
  MTL::Buffer* positions_buffer_ = nullptr;
  MTL::Buffer* index_buffer_ = nullptr;
  GltfModel model_{};
};
