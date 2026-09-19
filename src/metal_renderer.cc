#include "metal_renderer.h"

#include <simd/simd.h>

#include <MetalKit/MetalKit.hpp>
#include <cassert>
#include <cstring>

#include "executable_path.h"

namespace {
constexpr const char* kVertexShaderFunctionName = "VertexMain";
constexpr const char* kFragmentShaderFunctionName = "FragmentMain";
constexpr NS::UInteger kPositionsBufferIndex = 0;
constexpr NS::UInteger kColorsBufferIndex = 1;
}  // namespace

MetalRenderer::MetalRenderer(MTL::Device* device, MTK::View* view)
    : device_(device->retain()), view_(view) {
  command_queue_ = device_->newCommandQueue();
  assert(command_queue_ != nullptr && "Failed to create command queue.");

  NS::Error* shader_library_error = nullptr;
  std::string shader_library_path =
      (ExecutableDir() / "shaders.metallib").string();
  NS::URL* shader_library_url = NS::URL::fileURLWithPath(NS::String::string(
      shader_library_path.c_str(), NS::StringEncoding::UTF8StringEncoding));

  MTL::Library* shader_library =
      device_->newLibrary(shader_library_url, &shader_library_error);
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
      kColorPixelFormat);

  NS::Error* pipeline_state_error = nullptr;
  pipeline_state_ = device_->newRenderPipelineState(pipeline_descriptor,
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

  positions_buffer_ = device_->newBuffer(sizeof(positions), storage_mode);
  assert(positions_buffer_ != nullptr && "Failed to create positions buffer.");
  memcpy(positions_buffer_->contents(), positions.data(), sizeof(positions));

  colors_buffer_ = device_->newBuffer(sizeof(colors), storage_mode);
  assert(colors_buffer_ != nullptr && "Failed to create colors buffer.");
  memcpy(colors_buffer_->contents(), colors.data(), sizeof(colors));

  if (!device_->hasUnifiedMemory()) {
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

MetalRenderer::~MetalRenderer() {
  colors_buffer_->release();
  positions_buffer_->release();
  pipeline_state_->release();
  command_queue_->release();
  device_->release();
}

void MetalRenderer::Draw() {
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer* command_buffer = command_queue_->commandBuffer();
  MTL::RenderPassDescriptor* render_pass = view_->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* command_encoder =
      command_buffer->renderCommandEncoder(render_pass);

  command_encoder->setRenderPipelineState(pipeline_state_);
  command_encoder->setVertexBuffer(positions_buffer_, 0, kPositionsBufferIndex);
  command_encoder->setVertexBuffer(colors_buffer_, 0, kColorsBufferIndex);
  command_encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                  static_cast<NS::UInteger>(0),
                                  static_cast<NS::UInteger>(vertex_count_));

  command_encoder->endEncoding();
  command_buffer->presentDrawable(view_->currentDrawable());
  command_buffer->commit();

  pool->release();
}
