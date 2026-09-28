#ifndef METAL_CPP_SAMPLE_SCENES_MINIMAL_SCENE_H_
#define METAL_CPP_SAMPLE_SCENES_MINIMAL_SCENE_H_

#include <cstddef>

#include <Metal/Metal.hpp>

#include "platform/metal_ptr.h"
#include "scenes/scene.h"

class MinimalScene : public Scene {
 public:
  MinimalScene(const MinimalScene&) = delete;
  MinimalScene& operator=(const MinimalScene&) = delete;
  MinimalScene() = default;

  void Load(MTL::Device* device) override;

  void Update(float delta) override;

  void Draw(MTL::RenderCommandEncoder* command_encoder,
            const Camera& /*camera*/, std::size_t frame_index) override;

 private:
  PipelineStatePtr pipeline_state_;
  BufferPtr positions_buffer_;
  BufferPtr colors_buffer_;
  size_t vertex_count_ = 0;
};

#endif  // METAL_CPP_SAMPLE_SCENES_MINIMAL_SCENE_H_
