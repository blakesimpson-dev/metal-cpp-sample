#include "scenes/minimal_scene.h"

#include <simd/simd.h>

#include <array>
#include <iterator>

#include "renderer/camera.h"
#include "renderer/metal_utils.h"
#include "shaders/shader_types.h"

namespace {
constexpr const char* kVertexShaderFunctionName = "VertexMain";
constexpr const char* kFragmentShaderFunctionName = "FragmentMain";
}  // namespace

MinimalScene::~MinimalScene() {
  colors_buffer_->release();
  positions_buffer_->release();
  pipeline_state_->release();
}

void MinimalScene::Load(MTL::Device* device) {
  pipeline_state_ = CreateRenderPipelineState(device, kVertexShaderFunctionName,
                                              kFragmentShaderFunctionName);

  const std::array positions{simd::float3{-0.675F, 0.675F, 0.0F},
                             simd::float3{0.0F, -0.675F, 0.0F},
                             simd::float3{+0.675F, 0.675F, 0.0F}};

  vertex_count_ = std::size(positions);

  const std::array colors{simd::float3{1.0F, 0.0F, 0.0F},
                          simd::float3{0.0F, 1.0F, 0.0F},
                          simd::float3{0.0F, 0.0F, 1.0F}};

  positions_buffer_ = CreateBuffer(device, positions.data(),
                                   positions.size() * sizeof(simd::float3));

  colors_buffer_ =
      CreateBuffer(device, colors.data(), colors.size() * sizeof(simd::float3));
}

void MinimalScene::Update(float /*delta*/) {}

void MinimalScene::Draw(MTL::RenderCommandEncoder* command_encoder,
                        const Camera& /*camera*/) {
  command_encoder->setRenderPipelineState(pipeline_state_);
  command_encoder->setVertexBuffer(positions_buffer_, 0, kBufferIndexPositions);
  command_encoder->setVertexBuffer(colors_buffer_, 0, kBufferIndexColors);
  command_encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                  static_cast<NS::UInteger>(0),
                                  static_cast<NS::UInteger>(vertex_count_));
}
