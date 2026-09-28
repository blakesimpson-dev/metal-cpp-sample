#include "app/view_delegate.h"
#include "renderer/metal_renderer.h"
#include <memory>

ViewDelegate::ViewDelegate(MTL::Device* device, MTK::View* view, Scene* scene)
    : renderer_(std::make_unique<MetalRenderer>(device, view, scene)) {}

ViewDelegate::~ViewDelegate() = default;

void ViewDelegate::drawInMTKView(MTK::View* /*view*/) { renderer_->Draw(); }
