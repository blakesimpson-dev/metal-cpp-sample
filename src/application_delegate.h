#pragma once

#include <AppKit/AppKit.hpp>

namespace MTL {  // NOLINT(readability-identifier-naming)
class Device;
}
namespace MTK {  // NOLINT(readability-identifier-naming)
class View;
}
class ViewDelegate;

class ApplicationDelegate : public NS::ApplicationDelegate {
 public:
  ApplicationDelegate() = default;
  ApplicationDelegate(const ApplicationDelegate&) = delete;
  ApplicationDelegate& operator=(const ApplicationDelegate&) = delete;
  ~ApplicationDelegate() override;
  void applicationWillFinishLaunching(NS::Notification* notification) override;
  void applicationDidFinishLaunching(NS::Notification* notification) override;
  bool applicationShouldTerminateAfterLastWindowClosed(
      NS::Application* sender) override;

 private:
  MTL::Device* device_;
  NS::Window* window_;
  MTK::View* view_;
  ViewDelegate* view_delegate_;
};
