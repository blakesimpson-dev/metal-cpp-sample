#pragma once

#include <simd/simd.h>

#include <cstdint>
#include <filesystem>
#include <vector>

struct GltfModel {
  std::vector<simd::float3> positions;
  std::vector<simd::float3> normals;
  std::vector<std::uint32_t> indices;
  simd::float4x4 model_matrix;
  simd::float3 bounds_center;
  simd::float4 base_color;
  float metallic_factor;
  float bounds_radius;
};

GltfModel LoadGltfModel(const std::filesystem::path& model_file_path);
