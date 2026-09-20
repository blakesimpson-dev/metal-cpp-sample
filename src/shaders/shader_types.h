#pragma once

#include <simd/simd.h>

enum BufferIndex {  // NOLINT(performance-enum-size)
  kBufferIndexPositions = 0,
  kBufferIndexColors = 1,
  kBufferIndexTransform = 2,
};

struct Uniforms {
  simd::float4x4 mvp;
};
