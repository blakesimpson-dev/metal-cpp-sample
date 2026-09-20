#include "scenes/gltf_scene.h"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <numbers>

#include "app/application_delegate.h"
#include "platform/executable_path.h"
#include "renderer/math_utils.h"
#include "renderer/metal_utils.h"
#include "scenes/gltf_model.h"
#include "shaders/shader_types.h"

namespace {
constexpr const char* kModelSubdirectoryPath = "assets/exalted_orb";
constexpr const char* kModelFileName = "scene.gltf";
constexpr const char* kVertexShaderFunctionName = "GltfVertexMain";
constexpr const char* kFragmentShaderFunctionName = "GltfFragmentMain";
constexpr float kFovYRadians = 45.0F * std::numbers::pi_v<float> / 180.0F;
constexpr float kRotationSpeed = 0.25F;
const simd::float3 kWorldUp{0.0F, 1.0F, 0.0F};
const simd::float3 kLightDirection{
    simd::normalize(simd::float3{0.5F, 1.0F, 1.0F})};
const float kAmbientIntensity = 0.175F;
}  // namespace

GltfScene::~GltfScene() {
  depth_stencil_state_->release();
  normals_buffer_->release();
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
            << " indices, bounds center (" << model_.bounds_center.x << ", "
            << model_.bounds_center.y << ", " << model_.bounds_center.z
            << "), radius " << model_.bounds_radius << "\n";

  const float fit_distance =
      model_.bounds_radius / std::sin(kFovYRadians / 2.0F);
  const simd::float3 eye_position =
      model_.bounds_center + simd::float3{0.0F, 0.0F, fit_distance};

  view_matrix_ =
      DirectedViewMatrix(eye_position, model_.bounds_center, kWorldUp);
  projection_matrix_ = PerspectiveProjectionMatrix(
      kFovYRadians, ApplicationDelegate::kAspectRatio,
      fit_distance - (2 * model_.bounds_radius),
      fit_distance + (2 * model_.bounds_radius));

  pipeline_state_ = CreateRenderPipelineState(device, kVertexShaderFunctionName,
                                              kFragmentShaderFunctionName);

  positions_buffer_ =
      CreateBuffer(device, model_.positions.data(),
                   model_.positions.size() * sizeof(simd::float3));

  index_buffer_ = CreateBuffer(device, model_.indices.data(),
                               model_.indices.size() * sizeof(std::uint32_t));

  normals_buffer_ = CreateBuffer(device, model_.normals.data(),
                                 model_.normals.size() * sizeof(simd::float3));

  depth_stencil_state_ = CreateDepthStencilState(device);
}

void GltfScene::Update(float delta) {
  rotation_angle_ = std::fmod(rotation_angle_ + (kRotationSpeed * delta),
                              2.0F * std::numbers::pi_v<float>);
}

void GltfScene::Draw(MTL::RenderCommandEncoder* command_encoder) {
  const simd::float4x4 from_origin_matrix =
      TranslationMatrix(model_.bounds_center);
  const simd::float4x4 to_origin_matrix =
      TranslationMatrix(-model_.bounds_center);

  const simd::float4x4 delta_rotation_matrix =
      (XRotationMatrix(rotation_angle_ * 0.5F) *
       YRotationMatrix(rotation_angle_) *
       ZRotationMatrix(rotation_angle_ * 0.125F));

  const simd::float4x4 mvp_model_matrix =
      from_origin_matrix * delta_rotation_matrix * to_origin_matrix;

  const Uniforms uniforms{
      // NOTE: Order matters! mvp must always be evaluated in the following
      // order (reverse): projection * view * model
      .mvp = projection_matrix_ * view_matrix_ * mvp_model_matrix,
      .normal_matrix = NormalMatrix(mvp_model_matrix)};

  const FragmentUniforms fragment_uniforms{
      .base_color = model_.base_color,
      .light_direction = kLightDirection,
      .ambient_intensity = kAmbientIntensity,
  };

  command_encoder->setRenderPipelineState(pipeline_state_);
  command_encoder->setDepthStencilState(depth_stencil_state_);
  command_encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
  command_encoder->setCullMode(MTL::CullModeNone);

  command_encoder->setVertexBuffer(positions_buffer_, 0, kBufferIndexPositions);
  command_encoder->setVertexBuffer(normals_buffer_, 0, kBufferIndexNormals);

  command_encoder->setVertexBytes(&uniforms, sizeof(uniforms),
                                  kBufferIndexUniforms);
  command_encoder->setFragmentBytes(&fragment_uniforms,
                                    sizeof(fragment_uniforms),
                                    kBufferIndexFragmentUniforms);

  command_encoder->drawIndexedPrimitives(
      MTL::PrimitiveType::PrimitiveTypeTriangle,
      static_cast<NS::UInteger>(model_.indices.size()), MTL::IndexTypeUInt32,
      index_buffer_, 0);
}
