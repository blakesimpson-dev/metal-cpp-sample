#include "primitive_scene.h"

#include <simd/simd.h>

#include <cassert>
#include <cstring>

#include "executable_path.h"
#include "metal_renderer.h"

namespace {
constexpr const char* kVertexShaderFunctionName = "VertexMain";
constexpr const char* kFragmentShaderFunctionName = "FragmentMain";
constexpr NS::UInteger kPositionsBufferIndex = 0;
constexpr NS::UInteger kColorsBufferIndex = 1;
}  // namespace

PrimitiveScene::~PrimitiveScene() {
  colors_buffer_->release();
  positions_buffer_->release();
  pipeline_state_->release();
}

void PrimitiveScene::Load(MTL::Device* device) {
  NS::Error* shader_library_error = nullptr;
  std::string shader_library_path =
      (ExecutableDir() / "shaders.metallib").string();
  NS::URL* shader_library_url = NS::URL::fileURLWithPath(NS::String::string(
      shader_library_path.c_str(), NS::StringEncoding::UTF8StringEncoding));

  MTL::Library* shader_library =
      device->newLibrary(shader_library_url, &shader_library_error);
  assert(shader_library != nullptr && "Failed to create shader library.");

  MTL::Function* vertex_main = shader_library->newFunction(NS::String::string(
      kVertexShaderFunctionName, NS::StringEncoding::UTF8StringEncoding));
  assert(vertex_main != nullptr && "Failed to create shader vertex function.");

  MTL::Function* fragment_main = shader_library->newFunction(NS::String::string(
      kFragmentShaderFunctionName, NS::StringEncoding::UTF8StringEncoding));
  assert(fragment_main != nullptr &&
         "Failed to create shader fragment function.");

  MTL::RenderPipelineDescriptor* pipeline_descriptor =
      MTL::RenderPipelineDescriptor::alloc()->init();

  pipeline_descriptor->setVertexFunction(vertex_main);
  pipeline_descriptor->setFragmentFunction(fragment_main);
  pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(
      MetalRenderer::kColorPixelFormat);

  NS::Error* pipeline_state_error = nullptr;
  pipeline_state_ = device->newRenderPipelineState(pipeline_descriptor,
                                                   &pipeline_state_error);
  assert(pipeline_state_ != nullptr &&
         "Failed to create render pipeline state.");

  std::array positions{simd::float3{-0.675F, 0.675F, 0.0F},
                       simd::float3{0.0F, -0.675F, 0.0F},
                       simd::float3{+0.675F, 0.675F, 0.0F}};

  vertex_count_ = std::size(positions);

  std::array colors{simd::float3{1.0F, 0.0F, 0.0F},
                    simd::float3{0.0F, 1.0F, 0.0F},
                    simd::float3{0.0F, 0.0F, 1.0F}};

  // NOTE: ResourceStorageModeManaged path is untested (I don't have access to
  // a Mac with dedicated graphics..!)
  MTL::ResourceOptions storage_mode = device->hasUnifiedMemory()
                                          ? MTL::ResourceStorageModeShared
                                          : MTL::ResourceStorageModeManaged;

  positions_buffer_ = device->newBuffer(sizeof(positions), storage_mode);
  assert(positions_buffer_ != nullptr && "Failed to create positions buffer.");
  memcpy(positions_buffer_->contents(), positions.data(), sizeof(positions));

  colors_buffer_ = device->newBuffer(sizeof(colors), storage_mode);
  assert(colors_buffer_ != nullptr && "Failed to create colors buffer.");
  memcpy(colors_buffer_->contents(), colors.data(), sizeof(colors));

  if (!device->hasUnifiedMemory()) {
    positions_buffer_->didModifyRange(
        NS::Range::Make(0, positions_buffer_->length()));
    colors_buffer_->didModifyRange(
        NS::Range::Make(0, colors_buffer_->length()));
  }

  pipeline_descriptor->release();
  fragment_main->release();
  vertex_main->release();
  shader_library->release();
}

void PrimitiveScene::Update(float delta) {}

void PrimitiveScene::Draw(MTL::RenderCommandEncoder* command_encoder) {
  command_encoder->setRenderPipelineState(pipeline_state_);
  command_encoder->setVertexBuffer(positions_buffer_, 0, kPositionsBufferIndex);
  command_encoder->setVertexBuffer(colors_buffer_, 0, kColorsBufferIndex);
  command_encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                  static_cast<NS::UInteger>(0),
                                  static_cast<NS::UInteger>(vertex_count_));
}
