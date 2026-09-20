#include <metal_stdlib>
using namespace metal;

#include "shader_types.h"

struct VertexOut {
  float4 position [[position]];
  float3 world_normal;
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
  out.world_normal = uniforms.normal_matrix * normals[vertex_id];
  return out;
}

half4 fragment GltfFragmentMain(VertexOut in [[stage_in]],
                                constant FragmentUniforms& material
                                [[buffer(kBufferIndexFragmentUniforms)]]) {
  float3 normal = normalize(in.world_normal);
  float diffuse = fmax(dot(normal, material.light_direction), 0.0F);
  float3 color =
      material.base_color.rgb * (material.ambient_intensity + diffuse);
  return half4(half3(color), half(material.base_color.a));
}
