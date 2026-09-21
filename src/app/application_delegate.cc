#include "app/application_delegate.h"

#include <CoreFoundation/CFCGTypes.h>

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <cassert>
#include <iostream>
#include <memory>

#include "app/view_delegate.h"
#include "platform/metal_ptr.h"
#include "renderer/metal_renderer.h"

ApplicationDelegate::ApplicationDelegate(Scene* scene) : scene_(scene) {}

ApplicationDelegate::~ApplicationDelegate() {
  if (view_) {
    view_->setDelegate(nullptr);
  }
}

void ApplicationDelegate::applicationWillFinishLaunching(
    NS::Notification* /*notification*/) {
  NS::Application::sharedApplication()->setActivationPolicy(
      NS::ActivationPolicyRegular);
}

void ApplicationDelegate::applicationDidFinishLaunching(
    NS::Notification* /*notification*/) {
  device_ = NS::TransferPtr(MTL::CreateSystemDefaultDevice());
  assert(device_ && "Device creation failed: No system default device.");

  const CGRect content_rect{{kWindowOriginX, kWindowOriginY},
                            {kWindowWidth, kWindowHeight}};
  const NS::WindowStyleMask window_style_mask =
      NS::WindowStyleMaskTitled | NS::WindowStyleMaskClosable;
  const NS::BackingStoreType window_backing = NS::BackingStoreBuffered;
  const bool defer_onscreen_allocation = false;

  window_ =
      CreateMetalObject<NS::Window>(content_rect, window_style_mask,
                                    window_backing, defer_onscreen_allocation);
  assert(window_ && "Window creation failed: Could not initialize.");

  view_ = CreateMetalObject<MTK::View>(content_rect, device_.get());
  assert(view_ && "View creation failed: Could not initialize.");

  view_->setColorPixelFormat(MetalRenderer::kColorPixelFormat);
  view_->setDepthStencilPixelFormat(MetalRenderer::kDepthPixelFormat);

  view_delegate_ =
      std::make_unique<ViewDelegate>(device_.get(), view_.get(), scene_);
  view_->setDelegate(view_delegate_.get());

  window_->setContentView(view_.get());
  window_->setTitle(ToNsString(kWindowTitle));
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
