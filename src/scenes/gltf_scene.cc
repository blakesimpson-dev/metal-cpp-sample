#include "scenes/gltf_scene.h"

#include <cstdint>
#include <filesystem>
#include <iostream>

#include "platform/executable_path.h"
#include "renderer/metal_utils.h"
#include "scenes/gltf_model.h"
#include "shaders/shader_types.h"

namespace {
constexpr const char* kModelSubdirectoryPath = "assets/exalted_orb";
constexpr const char* kModelFileName = "scene.gltf";
constexpr const char* kVertexShaderFunctionName = "GltfVertexMain";
constexpr const char* kFragmentShaderFunctionName = "GltfFragmentMain";
}  // namespace

GltfScene::~GltfScene() {
  positions_buffer_->release();
  index_buffer_->release();
  pipeline_state_->release();
}

void GltfScene::Load(MTL::Device* device) {
  const std::filesystem::path model_directory_path =
      ExecutableDirectoryPath() / kModelSubdirectoryPath;
  const std::filesystem::path model_file_path =
      model_directory_path / kModelFileName;

  model_ = LoadGltfModel(model_file_path);
  std::cout << "Loaded '" << kModelFileName << "': " << model_.positions.size()
            << " vertices, " << model_.indices.size()
            << " indices, bounds centre (" << model_.bounds_centre.x << ", "
            << model_.bounds_centre.y << ", " << model_.bounds_centre.z
            << "), radius " << model_.bounds_radius << "\n";

  pipeline_state_ = CreateRenderPipelineState(device, kVertexShaderFunctionName,
                                              kFragmentShaderFunctionName);

  positions_buffer_ =
      CreateBuffer(device, model_.positions.data(),
                   model_.positions.size() * sizeof(simd::float3));

  index_buffer_ = CreateBuffer(device, model_.indices.data(),
                               model_.indices.size() * sizeof(std::uint32_t));
}

void GltfScene::Update(float delta) {}

void GltfScene::Draw(MTL::RenderCommandEncoder* command_encoder) {
  const float scale = 1.0F / model_.bounds_radius;
  const simd::float4x4 fit(
      simd::float4{scale, 0.0F, 0.0F, 0.0F},
      simd::float4{0.0F, scale, 0.0F, 0.0F},
      simd::float4{0.0F, 0.0F, 0.5F * scale, 0.0F},
      simd::float4{-scale * model_.bounds_centre.x,
                   -scale * model_.bounds_centre.y,
                   (-0.5F * scale * model_.bounds_centre.z) + 0.5F, 1.0F});
  const simd::float4x4 transform = simd_mul(fit, model_.model_matrix);

  command_encoder->setRenderPipelineState(pipeline_state_);
  command_encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
  command_encoder->setCullMode(MTL::CullModeNone);
  command_encoder->setVertexBuffer(positions_buffer_, 0, kBufferIndexPositions);
  command_encoder->setVertexBytes(&transform, sizeof(transform),
                                  kBufferIndexTransform);
  command_encoder->drawIndexedPrimitives(
      MTL::PrimitiveType::PrimitiveTypeTriangle,
      static_cast<NS::UInteger>(model_.indices.size()), MTL::IndexTypeUInt32,
      index_buffer_, 0);
}
