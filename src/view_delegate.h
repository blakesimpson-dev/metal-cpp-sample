#pragma once

#include <MetalKit/MetalKit.hpp>

class MetalRenderer;

class ViewDelegate : public MTK::ViewDelegate {
 public:
  ViewDelegate(const ViewDelegate&) = delete;
  ViewDelegate& operator=(const ViewDelegate&) = delete;
  ViewDelegate(MTL::Device* device, MTK::View* view);
  ~ViewDelegate() override;
  void drawInMTKView(MTK::View* view) override;

 private:
  MetalRenderer* renderer_;
};
