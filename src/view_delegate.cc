#include "view_delegate.h"

#include "metal_renderer.h"

ViewDelegate::ViewDelegate(MTL::Device* device, MTK::View* view)
    : renderer_(new MetalRenderer(device, view)) {}

ViewDelegate::~ViewDelegate() { delete renderer_; }

void ViewDelegate::drawInMTKView(MTK::View* view) { renderer_->Draw(); }
