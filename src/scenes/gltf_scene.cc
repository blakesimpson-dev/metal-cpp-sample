#include "scenes/gltf_scene.h"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <numbers>

#include "platform/executable_path.h"
#include "renderer/camera.h"
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
const simd::float3 kRotationSpeeds{0.25F, 0.125F, 0.09375F};
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
  model_rotation_angles_ += kRotationSpeeds * delta;
}

void GltfScene::Draw(MTL::RenderCommandEncoder* command_encoder,
                     const Camera& camera) {
  const simd::float4x4 from_origin_matrix =
      MakeTranslationMatrix(model_.bounds_center);
  const simd::float4x4 to_origin_matrix =
      MakeTranslationMatrix(-model_.bounds_center);

  const simd::float4x4 delta_rotation_matrix =
      (MakeXRotationMatrix(model_rotation_angles_.x) *
       MakeYRotationMatrix(model_rotation_angles_.y) *
       MakeZRotationMatrix(model_rotation_angles_.z));

  const simd::float4x4 model_world_matrix =
      from_origin_matrix * delta_rotation_matrix * to_origin_matrix *
      model_.model_matrix;

  const Uniforms uniforms{
      .mvp =
          camera.ProjectionMatrix() * camera.ViewMatrix() * model_world_matrix,
      .world_matrix = model_world_matrix,
      .normal_matrix = MakeNormalMatrix(model_world_matrix)};

  const FragmentUniforms fragment_uniforms{
      .base_color = model_.base_color,
      .light_direction = kLightDirection,
      .camera_position = camera.Position(),
      .sky_color = kSkyColor,
      .ground_color = kGroundColor,
      .ambient_intensity = kAmbientIntensity,
      .environment_intensity = kEnvironmentIntensity,
      .specular_intensity = kSpecularIntensity,
      .specular_exponent = kSpecularExponent,
      .metallic_factor = model_.metallic_factor,
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

void GltfScene::ConfigureCamera(Camera& camera) const {
  const float fit_distance =
      model_.bounds_radius / std::sin(kFovYRadians / 2.0F);
  const simd::float3 camera_position =
      model_.bounds_center + simd::float3{0.0F, 0.0F, fit_distance};

  camera.SetLookAt(camera_position, model_.bounds_center);
  camera.SetPerspective(kFovYRadians,
                        fit_distance - (2.0F * model_.bounds_radius),
                        fit_distance + (2.0F * model_.bounds_radius));
}

MTL::ClearColor GltfScene::SceneClearColor() const { return kSceneClearColor; }
