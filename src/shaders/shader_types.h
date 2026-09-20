#pragma once

#include <simd/simd.h>

enum BufferIndex {  // NOLINT(performance-enum-size)
  kBufferIndexPositions = 0,
  kBufferIndexColors = 1,
  kBufferIndexUniforms = 2,
  kBufferIndexNormals = 3,
  kBufferIndexFragmentUniforms = 4,
};

struct Uniforms {
  simd::float4x4 mvp;
  simd::float3x3 normal_matrix;
};

struct FragmentUniforms {
  simd::float4 base_color;
  simd::float3 light_direction;
  float ambient_intensity;
};
