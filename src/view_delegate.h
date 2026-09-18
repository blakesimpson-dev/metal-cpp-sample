#pragma once

#include <MetalKit/MetalKit.hpp>

class Renderer;

class ViewDelegate : public MTK::ViewDelegate {
 public:
  ViewDelegate(const ViewDelegate&) = delete;
  ViewDelegate& operator=(const ViewDelegate&) = delete;
  explicit ViewDelegate(MTL::Device* device);
  ~ViewDelegate() override;
  void drawInMTKView(MTK::View* view) override;

 private:
  Renderer* renderer_;
};
