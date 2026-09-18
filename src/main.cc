#include <AppKit/AppKit.hpp>

#include "application_delegate.h"

int main() {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  ApplicationDelegate application_delegate;

  NS::Application* shared_application = NS::Application::sharedApplication();
  shared_application->setDelegate(&application_delegate);
  shared_application->run();

  pool->release();
  return 0;
}
