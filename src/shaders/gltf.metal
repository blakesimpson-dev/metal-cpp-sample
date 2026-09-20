#include <metal_stdlib>
using namespace metal;

#include "shader_types.h"

struct VertexOut {
  float4 position [[position]];
  float3 normal;
};

VertexOut vertex GltfVertexMain(uint vertex_id [[vertex_id]],
                                device const float3* positions
                                [[buffer(kBufferIndexPositions)]],
                                constant Uniforms& uniforms
                                [[buffer(kBufferIndexUniforms)]],
                                device const float3* normals
                                [[buffer(kBufferIndexNormals)]]) {
  VertexOut out;
  out.position = uniforms.mvp * float4(positions[vertex_id], 1.0);
  out.normal = normals[vertex_id];
  return out;
}

half4 fragment GltfFragmentMain(VertexOut in [[stage_in]]) {
  return half4(half3(normalize(in.normal) * 0.5 + 0.5), 1.0);
}
