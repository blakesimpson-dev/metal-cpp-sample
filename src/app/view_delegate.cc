#include "app/view_delegate.h"

#include "renderer/metal_renderer.h"

ViewDelegate::ViewDelegate(MTL::Device* device, MTK::View* view, Scene* scene)
    : renderer_(new MetalRenderer(device, view, scene)) {}

ViewDelegate::~ViewDelegate() { delete renderer_; }

void ViewDelegate::drawInMTKView(MTK::View* /*view*/) { renderer_->Draw(); }
