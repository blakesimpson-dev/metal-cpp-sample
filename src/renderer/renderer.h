#pragma once

class Renderer {
 public:
  virtual ~Renderer() = default;

  virtual void Draw() = 0;
};
