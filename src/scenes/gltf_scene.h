#pragma once

#include <Metal/Metal.hpp>

#include "platform/metal_ptr.h"
#include "scenes/gltf_model.h"
#include "scenes/scene.h"

class GltfScene : public Scene {
 public:
  GltfScene(const GltfScene&) = delete;
  GltfScene& operator=(const GltfScene&) = delete;
  GltfScene() = default;

  void Load(MTL::Device* device) override;

  void Update(float delta) override;

  void Draw(MTL::RenderCommandEncoder* command_encoder,
            const Camera& camera) override;

  void ConfigureCamera(Camera& camera) const override;

  [[nodiscard]]
  MTL::ClearColor SceneClearColor() const override;

 private:
  PipelineStatePtr pipeline_state_;
  DepthStencilStatePtr depth_stencil_state_;

  BufferPtr positions_buffer_;
  BufferPtr index_buffer_;
  BufferPtr normals_buffer_;

  GltfModel model_{};
  simd::float3 model_rotation_angles_{};
};
