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
const simd::float3 kWorldUp{0.0F, 1.0F, 0.0F};
constexpr float kXRotationSpeed = 0.25F;
constexpr float kYRotationSpeed = 0.125F;
constexpr float kZRotationSpeed = 0.09375F;
const simd::float3 kLightDirection{
    simd::normalize(simd::float3{-0.625F, 0.625F, 0.375F})};
const simd::float3 kSkyColor{0.7F, 0.5F, 0.22F};
const simd::float3 kGroundColor{0.08F, 0.01F, 0.006F};
constexpr float kAmbientIntensity = 0.0375F;
constexpr float kEnvironmentIntensity = 1.25F;
constexpr float kSpecularIntensity = 6.0F;
constexpr float kSpecularExponent = 15.0F;

MTL::ClearColor DisplayClearColor(double red, double green, double blue) {
  return MTL::ClearColor::Make(std::pow(red, 2.2), std::pow(green, 2.2),
                               std::pow(blue, 2.2), 1.0);
}

const MTL::ClearColor kSceneClearColor = DisplayClearColor(0.09, 0.055, 0.035);
}  // namespace

GltfScene::~GltfScene() {
  normals_buffer_->release();
  index_buffer_->release();
  positions_buffer_->release();
  depth_stencil_state_->release();
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
  eye_position_ = model_.bounds_center + simd::float3{0.0F, 0.0F, fit_distance};

  view_matrix_ =
      DirectedViewMatrix(eye_position_, model_.bounds_center, kWorldUp);
  projection_matrix_ = PerspectiveProjectionMatrix(
      kFovYRadians, ApplicationDelegate::kAspectRatio,
      fit_distance - (2 * model_.bounds_radius),
      fit_distance + (2 * model_.bounds_radius));

  pipeline_state_ = CreateRenderPipelineState(device, kVertexShaderFunctionName,
                                              kFragmentShaderFunctionName);
  depth_stencil_state_ = CreateDepthStencilState(device);

  positions_buffer_ =
      CreateBuffer(device, model_.positions.data(),
                   model_.positions.size() * sizeof(simd::float3));

  index_buffer_ = CreateBuffer(device, model_.indices.data(),
                               model_.indices.size() * sizeof(std::uint32_t));

  normals_buffer_ = CreateBuffer(device, model_.normals.data(),
                                 model_.normals.size() * sizeof(simd::float3));
}

void GltfScene::Update(float delta) {
  x_rotation_angle_ = std::fmod(x_rotation_angle_ + (kXRotationSpeed * delta),
                                2.0F * std::numbers::pi_v<float>);
  y_rotation_angle_ = std::fmod(x_rotation_angle_ + (kYRotationSpeed * delta),
                                2.0F * std::numbers::pi_v<float>);
  z_rotation_angle_ = std::fmod(x_rotation_angle_ + (kZRotationSpeed * delta),
                                2.0F * std::numbers::pi_v<float>);
}

void GltfScene::Draw(MTL::RenderCommandEncoder* command_encoder) {
  const simd::float4x4 from_origin_matrix =
      TranslationMatrix(model_.bounds_center);
  const simd::float4x4 to_origin_matrix =
      TranslationMatrix(-model_.bounds_center);

  const simd::float4x4 delta_rotation_matrix =
      (XRotationMatrix(x_rotation_angle_) * YRotationMatrix(y_rotation_angle_) *
       ZRotationMatrix(z_rotation_angle_));

  const simd::float4x4 model_world_matrix =
      from_origin_matrix * delta_rotation_matrix * to_origin_matrix *
      model_.model_matrix;

  const Uniforms uniforms{
      // NOTE: Order matters! mvp must always be evaluated in the following
      // order (reverse): projection * view * model
      .mvp = projection_matrix_ * view_matrix_ * model_world_matrix,
      .world_matrix = model_world_matrix,
      .normal_matrix = NormalMatrix(model_world_matrix)};

  const FragmentUniforms fragment_uniforms{
      .base_color = model_.base_color,
      .light_direction = kLightDirection,
      .eye_position = eye_position_,
      .sky_color = kSkyColor,
      .ground_color = kGroundColor,
      .ambient_intensity = kAmbientIntensity,
      .environment_intensity = kEnvironmentIntensity,
      .specular_intensity = kSpecularIntensity,
      .specular_exponent = kSpecularExponent,
      .metallic_factor = model_.metallic_factor,
      .roughness_factor = model_.roughness_factor,
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

MTL::ClearColor GltfScene::ClearColor() const { return kSceneClearColor; }
