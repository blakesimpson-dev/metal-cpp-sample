#include <metal_stdlib>
using namespace metal;

#include "shader_types.h"

constant constexpr float kSpecularStrength = 3.0F;
constant constexpr float kDielectricSpecular = 0.04F;

struct VertexOut {
  float4 position [[position]];
  float3 world_position;
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
  const float4 position = float4(positions[vertex_id], 1.0F);

  out.position = uniforms.mvp * position;
  out.world_position = (uniforms.world_matrix * position).xyz;
  out.world_normal = uniforms.normal_matrix * normals[vertex_id];

  return out;
}

half4 fragment GltfFragmentMain(VertexOut in [[stage_in]],
                                constant FragmentUniforms& material
                                [[buffer(kBufferIndexFragmentUniforms)]]) {
  const float3 normal = normalize(in.world_normal);
  const float3 view_direction =
      normalize(material.eye_position - in.world_position);
  const float3 half_vector =
      normalize(material.light_direction + view_direction);

  const float diffuse = fmax(dot(normal, material.light_direction), 0.0F);
  const float specular_intensity =
      (diffuse > 0.0F) ? pow(fmax(dot(normal, half_vector), 0.0F),
                             material.specular_exponent)
                       : 0.0F;

  const float3 specular_color =
      mix(float3(kDielectricSpecular), material.base_color.rgb,
          material.metallic_factor);

  const float diffuse_weight = 1.0F - material.metallic_factor;
  const float3 color = material.base_color.rgb * (material.ambient_intensity +
                                                  diffuse * diffuse_weight) +
                       specular_color * specular_intensity * kSpecularStrength;

  return half4(half3(color), half(material.base_color.a));
}
