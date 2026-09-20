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

MeshInstance FindMeshInstance(fastgltf::Asset& asset) {
  assert(!asset.scenes.empty() && "Scene lookup failed: Model has no scenes.");

  const std::size_t gltf_scene_index = asset.defaultScene.value_or(0);
  // This represents the initial transform for the scene's root nodes
  const fastgltf::math::fmat4x4 root_matrix{};

  std::optional<MeshInstance> mesh_instance;
  fastgltf::iterateSceneNodes(
      asset, gltf_scene_index, root_matrix,
      // The matrix is already accumulated down the node chain
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

GltfModel LoadGltfModel(const std::filesystem::path& model_file_path) {
  auto model_load_result = fastgltf::GltfDataBuffer::FromPath(model_file_path);
  assert(model_load_result.error() == fastgltf::Error::None &&
         "Model read failed: Could not load the file.");
  fastgltf::GltfDataBuffer& model_data = model_load_result.get();

  fastgltf::Parser parser;
  auto model_parse_result =
      parser.loadGltf(model_data, model_file_path.parent_path(),
                      fastgltf::Options::LoadExternalBuffers);
  assert(model_parse_result.error() == fastgltf::Error::None &&
         "Model parse failed: Invalid glTF or missing buffer.");
  fastgltf::Asset& model_asset = model_parse_result.get();

  const MeshInstance mesh_instance = FindMeshInstance(model_asset);

  const std::size_t mesh_index = mesh_instance.mesh_index;
  assert(mesh_index < model_asset.meshes.size() &&
         "Mesh lookup failed: Mesh index out of range.");
  assert(model_asset.meshes[mesh_index].primitives.size() == 1 &&
         "Primitive lookup failed: Only one primitive per mesh is supported.");

  const fastgltf::Primitive& primitive =
      model_asset.meshes[mesh_index].primitives[0];
  assert(primitive.type == fastgltf::PrimitiveType::Triangles &&
         "Primitive check failed: Only triangle lists are supported.");

  GltfModel model{};
  model.positions = ReadVec3Attribute(model_asset, primitive, "POSITION");
  model.normals = ReadVec3Attribute(model_asset, primitive, "NORMAL");
  assert(model.normals.size() == model.positions.size() &&
         "Attribute check failed: POSITION and NORMAL counts differ.");

  model.indices = ReadIndices(model_asset, primitive);
  model.model_matrix = ToSimdFloat4x4(mesh_instance.world_matrix);

  const Bounds bounds = CalculateBounds(model.positions, model.model_matrix);
  model.bounds_centre = bounds.centre;
  model.bounds_radius = bounds.radius;

  return model;
}
