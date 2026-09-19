#pragma once

#include <simd/simd.h>

#include <Metal/Metal.hpp>
#include <cstdint>
#include <vector>

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
  std::vector<simd::float3> positions_;
  std::vector<simd::float3> normals_;
  std::vector<uint32_t> indices_;
  simd::float4x4 model_matrix_;
  simd::float3 bounds_centre_{};
  float bounds_radius_ = 0.0F;
};
