#ifndef METAL_CPP_SAMPLE_SCENES_SCENE_H_
#define METAL_CPP_SAMPLE_SCENES_SCENE_H_

#include <cstddef>

#include <Metal/Metal.hpp>

class Camera;

class Scene {
 public:
  virtual ~Scene() = default;
  virtual void Load(MTL::Device* device) = 0;
  virtual void Update(float delta) = 0;
  virtual void Draw(MTL::RenderCommandEncoder* command_encoder,
                    const Camera& camera, std::size_t frame_index) = 0;
  virtual void ConfigureCamera(Camera& /*camera*/) const {}
  [[nodiscard]]
  virtual MTL::ClearColor SceneClearColor() const {
    return MTL::ClearColor::Make(0.0, 0.0, 0.0, 1.0);
  }
};

#endif  // METAL_CPP_SAMPLE_SCENES_SCENE_H_
