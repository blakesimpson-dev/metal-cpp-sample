#include "app/application_delegate.h"

#include <CoreFoundation/CFCGTypes.h>

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <cassert>
#include <iostream>

#include "app/view_delegate.h"
#include "renderer/metal_renderer.h"

ApplicationDelegate::ApplicationDelegate(Scene* scene) : scene_(scene) {}

ApplicationDelegate::~ApplicationDelegate() {
  view_->release();
  window_->release();
  device_->release();
  delete view_delegate_;
}

void ApplicationDelegate::applicationWillFinishLaunching(
    NS::Notification* /*notification*/) {
  NS::Application::sharedApplication()->setActivationPolicy(
      NS::ActivationPolicyRegular);
}

void ApplicationDelegate::applicationDidFinishLaunching(
    NS::Notification* /*notification*/) {
  device_ = MTL::CreateSystemDefaultDevice();
  assert(device_ != nullptr &&
         "Device creation failed: No system default device.");

  const CGRect content_rect{{kWindowOriginX, kWindowOriginY},
                            {kWindowWidth, kWindowHeight}};
  const NS::WindowStyleMask window_style_mask =
      NS::WindowStyleMaskTitled | NS::WindowStyleMaskClosable;
  const NS::BackingStoreType window_backing = NS::BackingStoreBuffered;
  const bool defer_onscreen_allocation = false;

  window_ =
      NS::Window::alloc()->init(content_rect, window_style_mask, window_backing,
                                defer_onscreen_allocation);
  assert(window_ != nullptr && "Window creation failed: Could not initialize.");

  view_ = MTK::View::alloc()->init(content_rect, device_);
  assert(view_ != nullptr && "View creation failed: Could not initialize.");

  view_->setColorPixelFormat(MetalRenderer::kColorPixelFormat);
  view_->setDepthStencilPixelFormat(MetalRenderer::kDepthPixelFormat);

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
    NS::Application* /*sender*/) {
  return true;
}
