#pragma once

#include <Metal/Metal.hpp>

class Scene {
 public:
  virtual ~Scene() = default;
  virtual void Load(MTL::Device* device) = 0;
  virtual void Update(float delta) = 0;
  virtual void Draw(MTL::RenderCommandEncoder* command_encoder) = 0;
  [[nodiscard]]
  virtual MTL::ClearColor ClearColor() const {
    return MTL::ClearColor::Make(0.0, 0.0, 0.0, 1.0);
  }
};
