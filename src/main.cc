#include <AppKit/AppKit.hpp>

#include "application_delegate.h"
#include "primitive_scene.h"

int main() {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  Scene* scene = new PrimitiveScene();

  ApplicationDelegate application_delegate(scene);

  NS::Application* shared_application = NS::Application::sharedApplication();
  shared_application->setDelegate(&application_delegate);
  shared_application->run();

  delete scene;
  pool->release();
  return 0;
}
