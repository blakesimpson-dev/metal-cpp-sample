#include <AppKit/AppKit.hpp>
#include <cassert>
#include <memory>
#include <string_view>

#include "app/application_delegate.h"
#include "platform/metal_ptr.h"
#include "scenes/gltf_scene.h"
#include "scenes/minimal_scene.h"

namespace {
constexpr std::string_view kSceneFlag = "--scene=";
}

int main(int argc, char* argv[]) {
  AutoreleasePoolPtr pool = CreateMetalObject<NS::AutoreleasePool>();

  std::string_view scene_name = "gltf";
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg = argv[i];
    if (arg.starts_with(kSceneFlag)) {
      scene_name = arg.substr(kSceneFlag.size());
    }
  }

  std::unique_ptr<Scene> scene;
  if (scene_name == "minimal") {
    scene = std::make_unique<MinimalScene>();
  } else {
    assert(scene_name == "gltf" &&
           "Scene selection failed: Unknown scene name.");
    scene = std::make_unique<GltfScene>();
  }

  ApplicationDelegate application_delegate(scene.get());

  NS::Application* shared_application = NS::Application::sharedApplication();
  shared_application->setDelegate(&application_delegate);
  shared_application->run();

  return 0;
}
