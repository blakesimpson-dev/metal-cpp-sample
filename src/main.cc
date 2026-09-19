#include <AppKit/AppKit.hpp>
#include <cassert>
#include <string_view>

#include "app/application_delegate.h"
#include "scenes/gltf_scene.h"
#include "scenes/minimal_scene.h"

namespace {
constexpr std::string_view kSceneFlag = "--scene=";
}

int main(int argc, char* argv[]) {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  std::string_view scene_name = "gltf";
  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg.starts_with(kSceneFlag)) {
      scene_name = arg.substr(kSceneFlag.size());
    }
  }

  Scene* scene = nullptr;
  if (scene_name == "minimal") {
    scene = new MinimalScene();
  } else {
    assert(scene_name == "gltf" &&
           "Scene selection failed: Unknown scene name.");
    scene = new GltfScene();
  }

  ApplicationDelegate application_delegate(scene);

  NS::Application* shared_application = NS::Application::sharedApplication();
  shared_application->setDelegate(&application_delegate);
  shared_application->run();

  delete scene;
  pool->release();
  return 0;
}
