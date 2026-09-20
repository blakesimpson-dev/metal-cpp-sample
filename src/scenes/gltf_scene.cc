#include "scenes/gltf_scene.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fastgltf/core.hpp>
#include <fastgltf/math.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <string_view>
#include <vector>

#include "platform/executable_path.h"
#include "renderer/metal_utils.h"
#include "shaders/shader_types.h"

namespace {
constexpr const char* kModelSubdirectoryPath = "assets/exalted_orb";
constexpr const char* kModelFileName = "scene.gltf";
constexpr const char* kVertexShaderFunctionName = "GltfVertexMain";
constexpr const char* kFragmentShaderFunctionName = "GltfFragmentMain";

std::vector<simd::float3> ReadVec3Attribute(
    const fastgltf::Asset& asset, const fastgltf::Primitive& primitive,
    std::string_view attribute_name) {
  const fastgltf::Attribute* iterator = primitive.findAttribute(attribute_name);
  assert(iterator != primitive.attributes.end() &&
         "Attribute lookup failed: Requested attribute is missing.");

  const fastgltf::Accessor& accessor = asset.accessors[iterator->accessorIndex];
  std::vector<simd::float3> out(accessor.count);

  // Copying because fastgltf packs vec3s at 12 bytes
  fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
      asset, accessor, [&](fastgltf::math::fvec3 pos, std::size_t index) {
        out[index] = simd::float3{pos[0], pos[1], pos[2]};
      });

  return out;
}

std::vector<std::uint32_t> ReadIndices(const fastgltf::Asset& asset,
                                       const fastgltf::Primitive& primitive) {
  assert(primitive.indicesAccessor.has_value() &&
         "Index lookup failed: Primitive has no indices.");

  const fastgltf::Accessor& accessor =
      asset.accessors[*primitive.indicesAccessor];
  std::vector<std::uint32_t> out(accessor.count);

  fastgltf::copyFromAccessor<std::uint32_t>(asset, accessor, out.data());

  return out;
}

simd::float4x4 ToSimdFloat4x4(const fastgltf::math::fmat4x4& matrix) {
  // Both matrices are col major, elements copy across without a transpose
  simd::float4x4 out;
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      out.columns[col][row] = matrix[col][row];
    }
  }

  return out;
}

struct Bounds {
  simd::float3 centre;
  float radius;
};

Bounds CalculateBounds(const std::vector<simd::float3>& positions,
                       const simd::float4x4& model_matrix) {
  assert(!positions.empty() && "Bounds calculation failed: No position data.");

  // w = 1.0F marks a point so the matrix's translation applies!
  const auto to_world = [&](const simd::float3& position) {
    const simd::float4 world = simd_mul(
        model_matrix, simd::float4{position.x, position.y, position.z, 1.0F});
    return simd::float3{world.x, world.y, world.z};
  };

  simd::float3 min_corner{std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max()};

  // lowest(), not min()! min() is the smallest positive float..
  simd::float3 max_corner{std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest()};

  for (const simd::float3& pos : positions) {
    const simd::float3 world_position = to_world(pos);

    min_corner = simd_min(min_corner, world_position);
    max_corner = simd_max(max_corner, world_position);
  }

  const simd::float3 centre = (min_corner + max_corner) * 0.5F;

  float radius = 0.0F;
  // The radius needs the final centre, so it takes a second pass
  for (const simd::float3& position : positions) {
    radius = std::max(radius, simd_length(to_world(position) - centre));
  }

  return Bounds{.centre = centre, .radius = radius};
}
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

  auto model_load_result = fastgltf::GltfDataBuffer::FromPath(model_file_path);
  assert(model_load_result.error() == fastgltf::Error::None &&
         "Model read failed: Could not load the file.");
  fastgltf::GltfDataBuffer& model_data = model_load_result.get();

  fastgltf::Parser parser;
  auto model_parse_result = parser.loadGltf(
      model_data, model_directory_path, fastgltf::Options::LoadExternalBuffers);
  assert(model_parse_result.error() == fastgltf::Error::None &&
         "Model parse failed: Invalid glTF or missing buffer.");
  fastgltf::Asset& model_asset = model_parse_result.get();

  assert(!model_asset.scenes.empty() &&
         "Scene lookup failed: Model has no scenes.");

  const std::size_t gltf_scene_index = model_asset.defaultScene.value_or(0);
  // This represents the initial transform for the scene's root nodes
  const fastgltf::math::fmat4x4 root_matrix{};

  struct SceneTraversalResult {
    std::size_t mesh_index;
    fastgltf::math::fmat4x4 world_matrix;
  };

  std::optional<SceneTraversalResult> scene_traversal_result;
  fastgltf::iterateSceneNodes(
      model_asset, gltf_scene_index, root_matrix,
      // The matrix is already accumulated down the node chain
      [&](fastgltf::Node& node, const fastgltf::math::fmat4x4& matrix) {
        if (node.meshIndex.has_value()) {
          assert(!scene_traversal_result.has_value() &&
                 "Scene node traversal failed: More than one mesh node found.");
          scene_traversal_result = SceneTraversalResult{
              .mesh_index = *node.meshIndex, .world_matrix = matrix};
        }
      });

  assert(scene_traversal_result.has_value() &&
         "Scene node traversal failed: No mesh node found.");

  const std::size_t mesh_index = scene_traversal_result->mesh_index;
  assert(mesh_index < model_asset.meshes.size() &&
         "Mesh lookup failed: Mesh index out of range.");
  assert(model_asset.meshes[mesh_index].primitives.size() == 1 &&
         "Primitive lookup failed: Only one primitive per mesh is supported.");

  const fastgltf::math::fmat4x4 world_matrix =
      scene_traversal_result->world_matrix;

  const fastgltf::Primitive& primitive =
      model_asset.meshes[mesh_index].primitives[0];
  assert(primitive.type == fastgltf::PrimitiveType::Triangles &&
         "Primitive check failed: Only triangle lists are supported.");

  positions_ = ReadVec3Attribute(model_asset, primitive, "POSITION");
  normals_ = ReadVec3Attribute(model_asset, primitive, "NORMAL");
  assert(normals_.size() == positions_.size() &&
         "Attribute check failed: POSITION and NORMAL counts differ.");

  indices_ = ReadIndices(model_asset, primitive);
  model_matrix_ = ToSimdFloat4x4(world_matrix);

  const Bounds bounds = CalculateBounds(positions_, model_matrix_);
  bounds_centre_ = bounds.centre;
  bounds_radius_ = bounds.radius;

  std::cout << "Loaded '" << kModelFileName << "': " << positions_.size()
            << " vertices, " << indices_.size() << " indices, bounds centre ("
            << bounds_centre_.x << ", " << bounds_centre_.y << ", "
            << bounds_centre_.z << "), radius " << bounds_radius_ << "\n";

  pipeline_state_ = CreateRenderPipelineState(device, kVertexShaderFunctionName,
                                              kFragmentShaderFunctionName);

  positions_buffer_ = CreateBuffer(device, positions_.data(),
                                   positions_.size() * sizeof(simd::float3));

  index_buffer_ = CreateBuffer(device, indices_.data(),
                               indices_.size() * sizeof(std::uint32_t));
}

void GltfScene::Update(float delta) {}

void GltfScene::Draw(MTL::RenderCommandEncoder* command_encoder) {
  const float scale = 1.0F / bounds_radius_;
  const simd::float4x4 fit(
      simd::float4{scale, 0.0F, 0.0F, 0.0F},
      simd::float4{0.0F, scale, 0.0F, 0.0F},
      simd::float4{0.0F, 0.0F, 0.5F * scale, 0.0F},
      simd::float4{-scale * bounds_centre_.x, -scale * bounds_centre_.y,
                   (-0.5F * scale * bounds_centre_.z) + 0.5F, 1.0F});
  const simd::float4x4 transform = simd_mul(fit, model_matrix_);

  command_encoder->setRenderPipelineState(pipeline_state_);
  command_encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
  command_encoder->setCullMode(MTL::CullModeNone);
  command_encoder->setVertexBuffer(positions_buffer_, 0, kBufferIndexPositions);
  command_encoder->setVertexBytes(&transform, sizeof(transform),
                                  kBufferIndexTransform);
  command_encoder->drawIndexedPrimitives(
      MTL::PrimitiveType::PrimitiveTypeTriangle,
      static_cast<NS::UInteger>(indices_.size()), MTL::IndexTypeUInt32,
      index_buffer_, 0);
}
