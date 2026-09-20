#include <metal_stdlib>
using namespace metal;

#include "shader_types.h"

struct VertexOut {
  float4 position [[position]];
};

VertexOut vertex GltfVertexMain(uint vertex_id [[vertex_id]],
                                device const float3* positions
                                [[buffer(kBufferIndexPositions)]],
                                constant Uniforms& uniforms
                                [[buffer(kBufferIndexUniforms)]]) {
  VertexOut out;
  out.position = uniforms.mvp * float4(positions[vertex_id], 1.0);
  return out;
}

half4 fragment GltfFragmentMain() { return half4(0.8, 0.6, 0.2, 1.0); }
