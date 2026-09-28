
#ifndef METAL_CPP_SAMPLE_SCENES_GLTF_SCENE_H_
#define METAL_CPP_SAMPLE_SCENES_GLTF_SCENE_H_

#include "platform/metal_ptr.h"
#include "scenes/gltf_model.h"
#include "scenes/scene.h"
#include <Metal/Metal.hpp>

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

#endif  // METAL_CPP_SAMPLE_SCENES_GLTF_SCENE_H_
