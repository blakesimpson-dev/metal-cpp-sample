#pragma once

#include <AppKit/AppKit.hpp>
#include <memory>

#include "platform/metal_ptr.h"

class Scene;
class ViewDelegate;

class ApplicationDelegate : public NS::ApplicationDelegate {
 public:
  static constexpr const char* kWindowTitle = "Metal-cpp Sample";
  static constexpr double kWindowOriginX = 128.0;
  static constexpr double kWindowOriginY = 128.0;
  static constexpr double kWindowWidth = 1024.0;
  static constexpr double kWindowHeight = 1024.0;

  ApplicationDelegate(const ApplicationDelegate&) = delete;
  ApplicationDelegate& operator=(const ApplicationDelegate&) = delete;
  explicit ApplicationDelegate(Scene* scene);

  ~ApplicationDelegate() override;

  void applicationWillFinishLaunching(
      NS::Notification* /*notification*/) override;

  void applicationDidFinishLaunching(
      NS::Notification* /*notification*/) override;

  bool applicationShouldTerminateAfterLastWindowClosed(
      NS::Application* /*sender*/) override;

 private:
  DevicePtr device_;
  WindowPtr window_;
  ViewPtr view_;

  Scene* scene_;
  std::unique_ptr<ViewDelegate> view_delegate_;
};
