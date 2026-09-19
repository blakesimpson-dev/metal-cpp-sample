#include "app/application_delegate.h"

#include <CoreFoundation/CFCGTypes.h>

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <cassert>
#include <iostream>

#include "app/view_delegate.h"
#include "renderer/metal_renderer.h"

namespace {
constexpr const char* kWindowTitle = "Metal-cpp Sample";
}

ApplicationDelegate::ApplicationDelegate(Scene* scene) : scene_(scene) {}

ApplicationDelegate::~ApplicationDelegate() {
  view_->release();
  window_->release();
  device_->release();
  delete view_delegate_;
}

void ApplicationDelegate::applicationWillFinishLaunching(
    NS::Notification* notification) {
  NS::Application::sharedApplication()->setActivationPolicy(
      NS::ActivationPolicyRegular);
}

void ApplicationDelegate::applicationDidFinishLaunching(
    NS::Notification* notification) {
  device_ = MTL::CreateSystemDefaultDevice();
  assert(device_ != nullptr && "Failed to create device.");

  CGRect content_rect = (CGRect){{128.0, 128.0}, {1024.0, 1024.0}};
  NS::WindowStyleMask window_style_mask =
      NS::WindowStyleMaskTitled | NS::WindowStyleMaskClosable;
  NS::BackingStoreType window_backing = NS::BackingStoreBuffered;
  bool defer_onscreen_allocation = false;

  window_ =
      NS::Window::alloc()->init(content_rect, window_style_mask, window_backing,
                                defer_onscreen_allocation);
  assert(window_ != nullptr && "Failed to create window.");

  view_ = MTK::View::alloc()->init(content_rect, device_);
  assert(view_ != nullptr && "Failed to create view.");

  view_->setColorPixelFormat(MetalRenderer::kColorPixelFormat);
  view_->setClearColor(MTL::ClearColor::Make(0.0, 0.0, 0.0, 1.0));

  view_delegate_ = new ViewDelegate(device_, view_, scene_);
  view_->setDelegate(view_delegate_);

  window_->setContentView(view_);
  window_->setTitle(
      NS::String::string(kWindowTitle, NS::StringEncoding::UTF8StringEncoding));
  window_->makeKeyAndOrderFront(nullptr);

  NS::Application::sharedApplication()->activateIgnoringOtherApps(true);

  const char* device_name = device_->name()->utf8String();
  std::cout << "'" << kWindowTitle << "' running using " << device_name << " ("
            << (device_->hasUnifiedMemory() ? "Integrated" : "Dedicated")
            << " GPU)\n";
}

bool ApplicationDelegate::applicationShouldTerminateAfterLastWindowClosed(
    NS::Application* sender) {
  return true;
}
