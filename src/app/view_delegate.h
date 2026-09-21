#pragma once

#include <MetalKit/MetalKit.hpp>
#include <memory>

class MetalRenderer;
class Scene;

class ViewDelegate : public MTK::ViewDelegate {
 public:
  ViewDelegate(const ViewDelegate&) = delete;
  ViewDelegate& operator=(const ViewDelegate&) = delete;
  ViewDelegate(MTL::Device* device, MTK::View* view, Scene* scene);

  ~ViewDelegate() override;

  void drawInMTKView(MTK::View* view) override;

 private:
  std::unique_ptr<MetalRenderer> renderer_;
};
