#pragma once

#include <simd/simd.h>

enum BufferIndex : unsigned char {
  kBufferIndexPositions = 0,
  kBufferIndexColors = 1,
  kBufferIndexUniforms = 2,
  kBufferIndexNormals = 3,
  kBufferIndexFragmentUniforms = 4,
};

struct Uniforms {
  simd::float4x4 mvp;
  simd::float4x4 world_matrix;
  simd::float3x3 normal_matrix;
};

struct FragmentUniforms {
  simd::float4 base_color;
  simd::float3 light_direction;
  simd::float3 camera_position;
  simd::float3 sky_color;
  simd::float3 ground_color;
  float ambient_intensity;
  float environment_intensity;
  float specular_intensity;
  float specular_exponent;
  float metallic_factor;
  float bump_strength;
  float bump_frequency;
};
