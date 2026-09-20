#include "gltf_model.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fastgltf/core.hpp>
#include <fastgltf/math.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <filesystem>
#include <limits>
#include <optional>
#include <string_view>
#include <vector>

namespace {
struct MeshInstance {
  std::size_t mesh_index;
  fastgltf::math::fmat4x4 world_matrix;
};

simd::float3 TransformPoint(const simd::float4x4& matrix,
                            const simd::float3& point) {
  const simd::float4 result =
      simd_mul(matrix, simd::float4{point.x, point.y, point.z, 1.0F});
  return simd::float3{result.x, result.y, result.z};
}

MeshInstance FindMeshInstance(fastgltf::Asset& asset) {
  assert(!asset.scenes.empty() && "Scene lookup failed: Model has no scenes.");

  const std::size_t gltf_scene_index = asset.defaultScene.value_or(0);
  const fastgltf::math::fmat4x4 root_matrix{};

  std::optional<MeshInstance> mesh_instance;
  fastgltf::iterateSceneNodes(
      asset, gltf_scene_index, root_matrix,
      [&](fastgltf::Node& node, const fastgltf::math::fmat4x4& matrix) {
        if (node.meshIndex.has_value()) {
          assert(!mesh_instance.has_value() &&
                 "Scene node traversal failed: More than one mesh instance "
                 "found.");
          mesh_instance = MeshInstance{.mesh_index = *node.meshIndex,
                                       .world_matrix = matrix};
        }
      });

  assert(mesh_instance.has_value() &&
         "Scene node traversal failed: No mesh instance found.");

  return *mesh_instance;
}

std::vector<simd::float3> ReadVec3Attribute(
    const fastgltf::Asset& asset, const fastgltf::Primitive& primitive,
    std::string_view attribute_name) {
  const fastgltf::Attribute* iterator = primitive.findAttribute(attribute_name);
  assert(iterator != primitive.attributes.end() &&
         "Attribute lookup failed: Requested attribute is missing.");

  const fastgltf::Accessor& accessor = asset.accessors[iterator->accessorIndex];
  std::vector<simd::float3> out(accessor.count);

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
  simd::float4x4 out;
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      out.columns[col][row] = matrix[col][row];
    }
  }

  return out;
}

struct Bounds {
  simd::float3 center;
  float radius;
};

Bounds ComputeBounds(const std::vector<simd::float3>& positions,
                     const simd::float4x4& model_matrix) {
  assert(!positions.empty() && "Bounds calculation failed: No position data.");

  simd::float3 min_corner{std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max()};

  simd::float3 max_corner{std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest()};

  for (const simd::float3& position : positions) {
    const simd::float3 world_position = TransformPoint(model_matrix, position);

    min_corner = simd_min(min_corner, world_position);
    max_corner = simd_max(max_corner, world_position);
  }

  const simd::float3 center = (min_corner + max_corner) * 0.5F;
  float radius = 0.0F;
  for (const simd::float3& position : positions) {
    radius = std::max(
        radius, simd_length(TransformPoint(model_matrix, position) - center));
  }

  return Bounds{.center = center, .radius = radius};
}

const fastgltf::PBRData& ReadPbrData(const fastgltf::Asset& asset,
                                     const fastgltf::Primitive& primitive) {
  assert(primitive.materialIndex.has_value() &&
         "Material lookup failed: Primitive has no material.");
  assert(*primitive.materialIndex < asset.materials.size() &&
         "Material lookup failed: Material index out of range.");

  return asset.materials[*primitive.materialIndex].pbrData;
}
}  // namespace

GltfModel LoadGltfModel(const std::filesystem::path& model_file_path) {
  fastgltf::Expected<fastgltf::GltfDataBuffer> asset_load_result =
      fastgltf::GltfDataBuffer::FromPath(model_file_path);

  assert(asset_load_result.error() == fastgltf::Error::None &&
         "Model read failed: Could not load the file.");
  fastgltf::GltfDataBuffer& asset_data = asset_load_result.get();

  fastgltf::Parser gltf_parser;
  fastgltf::Expected<fastgltf::Asset> asset_parse_result =
      gltf_parser.loadGltf(asset_data, model_file_path.parent_path(),
                           fastgltf::Options::LoadExternalBuffers);

  assert(asset_parse_result.error() == fastgltf::Error::None &&
         "Model parse failed: Invalid glTF or missing buffer.");
  fastgltf::Asset& asset = asset_parse_result.get();

  const MeshInstance mesh_instance = FindMeshInstance(asset);

  const std::size_t mesh_index = mesh_instance.mesh_index;
  assert(mesh_index < asset.meshes.size() &&
         "Mesh lookup failed: Mesh index out of range.");
  assert(asset.meshes[mesh_index].primitives.size() == 1 &&
         "Primitive lookup failed: Only one primitive per mesh is supported.");

  const fastgltf::Primitive& primitive = asset.meshes[mesh_index].primitives[0];
  assert(primitive.type == fastgltf::PrimitiveType::Triangles &&
         "Primitive check failed: Only triangle lists are supported.");

  GltfModel model{};
  model.positions = ReadVec3Attribute(asset, primitive, "POSITION");
  model.normals = ReadVec3Attribute(asset, primitive, "NORMAL");
  assert(model.normals.size() == model.positions.size() &&
         "Attribute check failed: POSITION and NORMAL counts differ.");

  model.indices = ReadIndices(asset, primitive);
  model.model_matrix = ToSimdFloat4x4(mesh_instance.world_matrix);

  const Bounds bounds = ComputeBounds(model.positions, model.model_matrix);
  model.bounds_center = bounds.center;
  model.bounds_radius = bounds.radius;

  const fastgltf::PBRData& pbr_data = ReadPbrData(asset, primitive);

  model.base_color = simd::float4{
      pbr_data.baseColorFactor[0],
      pbr_data.baseColorFactor[1],
      pbr_data.baseColorFactor[2],
      pbr_data.baseColorFactor[3],
  };

  model.metallic_factor = pbr_data.metallicFactor;
  model.roughness_factor = pbr_data.roughnessFactor;

  return model;
}
