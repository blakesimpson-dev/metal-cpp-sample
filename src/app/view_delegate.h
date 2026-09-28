
#ifndef METAL_CPP_SAMPLE_APP_VIEW_DELEGATE_H_
#define METAL_CPP_SAMPLE_APP_VIEW_DELEGATE_H_

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

#endif  // METAL_CPP_SAMPLE_APP_VIEW_DELEGATE_H_
