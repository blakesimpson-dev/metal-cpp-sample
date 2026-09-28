#ifndef METAL_CPP_SAMPLE_SCENES_GLTF_SCENE_H_
#define METAL_CPP_SAMPLE_SCENES_GLTF_SCENE_H_

#include <array>
#include <cstddef>

#include <Metal/Metal.hpp>

#include "platform/metal_ptr.h"
#include "renderer/frames_in_flight.h"
#include "scenes/gltf_model.h"
#include "scenes/scene.h"

class GltfScene : public Scene {
 public:
  GltfScene(const GltfScene&) = delete;
  GltfScene& operator=(const GltfScene&) = delete;
  GltfScene() = default;
  void Load(MTL::Device* device) override;
  void Update(float delta) override;
  void Draw(MTL::RenderCommandEncoder* command_encoder, const Camera& camera,
            std::size_t frame_index) override;
  void ConfigureCamera(Camera& camera) const override;
  [[nodiscard]]
  MTL::ClearColor SceneClearColor() const override;

 private:
  PipelineStatePtr pipeline_state_;
  DepthStencilStatePtr depth_stencil_state_;
  BufferPtr positions_buffer_;
  BufferPtr index_buffer_;
  BufferPtr normals_buffer_;
  std::array<BufferPtr, kMaxFramesInFlight> uniform_buffers_;
  std::array<BufferPtr, kMaxFramesInFlight> fragment_uniform_buffers_;
  GltfModel model_{};
  float yaw_radians_ = 0.0F;
  float tilt_phase_radians_ = 0.0F;
};

#endif  // METAL_CPP_SAMPLE_SCENES_GLTF_SCENE_H_
