#pragma once

#include <simd/simd.h>

enum BufferIndex {  // NOLINT(performance-enum-size)
  kBufferIndexPositions = 0,
  kBufferIndexColors = 1,
  kBufferIndexUniforms = 2,
  kBufferIndexNormals = 3,
};

struct Uniforms {
  simd::float4x4 mvp;
};
