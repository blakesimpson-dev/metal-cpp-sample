
#ifndef METAL_CPP_SAMPLE_RENDERER_RENDERER_H_
#define METAL_CPP_SAMPLE_RENDERER_RENDERER_H_

class Renderer {
 public:
  virtual ~Renderer() = default;

  virtual void Draw() = 0;
};

#endif  // METAL_CPP_SAMPLE_RENDERER_RENDERER_H_
