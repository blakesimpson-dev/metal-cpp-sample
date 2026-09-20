#pragma once

#include <AppKit/AppKit.hpp>

namespace MTL {  // NOLINT(readability-identifier-naming)
class Device;
}

namespace MTK {  // NOLINT(readability-identifier-naming)
class View;
}

class Scene;
class ViewDelegate;

class ApplicationDelegate : public NS::ApplicationDelegate {
 public:
  static constexpr const char* kWindowTitle = "Metal-cpp Sample";
  static constexpr double kWindowOriginX = 128.0;
  static constexpr double kWindowOriginY = 128.0;
  static constexpr double kWindowWidth = 1024.0;
  static constexpr double kWindowHeight = 1024.0;
  static constexpr float kAspectRatio =
      static_cast<float>(kWindowWidth / kWindowHeight);

  ApplicationDelegate(const ApplicationDelegate&) = delete;
  ApplicationDelegate& operator=(const ApplicationDelegate&) = delete;
  explicit ApplicationDelegate(Scene* scene);
  ~ApplicationDelegate() override;
  void applicationWillFinishLaunching(NS::Notification* notification) override;
  void applicationDidFinishLaunching(NS::Notification* notification) override;
  bool applicationShouldTerminateAfterLastWindowClosed(
      NS::Application* sender) override;

 private:
  MTL::Device* device_ = nullptr;
  NS::Window* window_ = nullptr;
  MTK::View* view_ = nullptr;
  Scene* scene_;
  ViewDelegate* view_delegate_ = nullptr;
};
