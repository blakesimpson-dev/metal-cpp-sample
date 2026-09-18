#include "view_delegate.h"

#include "renderer.h"

ViewDelegate::ViewDelegate(MTL::Device* device)
    : renderer_(new Renderer(device)) {}

ViewDelegate::~ViewDelegate() { delete renderer_; }

void ViewDelegate::drawInMTKView(MTK::View* view) { renderer_->Draw(view); }
