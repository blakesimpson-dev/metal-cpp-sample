#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

#include <AppKit/AppKit.hpp>

#include "app/application_delegate.h"
#include "platform/fatal_error.h"
#include "platform/metal_ptr.h"
#include "renderer/metal_renderer.h"
#include "scenes/gltf_scene.h"
#include "scenes/minimal_scene.h"

namespace {
constexpr std::string_view kSceneFlag = "--scene=";
constexpr std::string_view kMsaaFlag = "--msaa=";

struct Options {
  std::string_view scene_name = "gltf";
  bool msaa_enabled = true;
};

Options ParseOptions(std::span<char*> args) {
  Options options;
  for (const std::string_view arg : args.subspan(1)) {
    if (arg.starts_with(kSceneFlag)) {
      options.scene_name = arg.substr(kSceneFlag.size());
      Check(options.scene_name == "gltf" || options.scene_name == "minimal",
            "Argument check failed: --scene must be gltf or minimal.");
    } else if (arg.starts_with(kMsaaFlag)) {
      const std::string_view value = arg.substr(kMsaaFlag.size());
      Check(value == "on" || value == "off",
            "Argument check failed: --msaa must be on or off.");
      options.msaa_enabled = value == "on";
    } else {
      FatalError(
          "Argument check failed: Unknown option. Usage: metal-cpp-sample "
          "[--scene=gltf|minimal] [--msaa=on|off]");
    }
  }
  return options;
}
}  // namespace

int main(int argc, char* argv[]) {
  AutoreleasePoolPtr pool = CreateMetalObject<NS::AutoreleasePool>();

  const Options options = ParseOptions({argv, static_cast<std::size_t>(argc)});
  MetalRenderer::SetMsaaEnabled(options.msaa_enabled);

  std::unique_ptr<Scene> scene;
  if (options.scene_name == "minimal") {
    scene = std::make_unique<MinimalScene>();
  } else {
    scene = std::make_unique<GltfScene>();
  }

  ApplicationDelegate application_delegate(scene.get());
  NS::Application* shared_application = NS::Application::sharedApplication();
  shared_application->setDelegate(&application_delegate);
  shared_application->run();

  return 0;
}
