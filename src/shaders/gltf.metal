#include <metal_stdlib>
using namespace metal;

#include "shader_types.h"

constant constexpr float kDielectricSpecular = 0.04F;
constant constexpr float kFresnelPower = 5.0F;
constant constexpr float kHorizonBlendWidth = 0.08F;

struct VertexOut {
  float3 model_position;
  float3 model_normal;
  float4 clip_position [[position]];
  float3 world_position;
  float3 world_normal;
};

VertexOut vertex GltfVertexMain(uint vertex_id [[vertex_id]],
                                device const float3* model_positions
                                [[buffer(kBufferIndexPositions)]],
                                constant Uniforms& uniforms
                                [[buffer(kBufferIndexUniforms)]],
                                device const float3* model_normals
                                [[buffer(kBufferIndexNormals)]]) {
  VertexOut out;
  out.model_position = model_positions[vertex_id];
  out.model_normal = model_normals[vertex_id];
  out.clip_position = uniforms.mvp * float4(out.model_position, 1.0F);
  out.world_position =
      (uniforms.world_matrix * float4(out.model_position, 1.0F)).xyz;
  out.world_normal = uniforms.normal_matrix * out.model_normal;
  return out;
}

float3 SampleEnvironment(float3 direction,
                         constant FragmentUniforms& material) {
  return mix(material.ground_color, material.sky_color,
             smoothstep(-kHorizonBlendWidth, kHorizonBlendWidth, direction.y));
}

half4 fragment GltfFragmentMain(VertexOut in [[stage_in]],
                                constant FragmentUniforms& material
                                [[buffer(kBufferIndexFragmentUniforms)]]) {
  const float3 normal = normalize(in.world_normal);
  const float3 view_direction =
      normalize(material.camera_position - in.world_position);
  const float3 half_vector =
      normalize(material.light_direction + view_direction);

  const float diffuse = fmax(dot(normal, material.light_direction), 0.0F);
  const float specular_term =
      pow(fmax(dot(normal, half_vector), 0.0F), material.specular_exponent) *
      (diffuse > 0.0F ? 1.0F : 0.0F);

  const float3 specular_color =
      mix(float3(kDielectricSpecular), material.base_color.rgb,
          material.metallic_factor);

  const float3 reflection_direction = reflect(-view_direction, normal);
  const float3 fresnel =
      specular_color +
      (float3(1.0F) - specular_color) *
          pow(1.0F - fmax(dot(normal, view_direction), 0.0F), kFresnelPower);

  const float diffuse_weight = 1.0F - material.metallic_factor;
  const float3 color =
      material.base_color.rgb *
          (material.ambient_intensity + diffuse * diffuse_weight) +
      (specular_color * specular_term * material.specular_intensity) +
      (fresnel * SampleEnvironment(reflection_direction, material) *
       material.environment_intensity);

  return half4(half3(color), half(material.base_color.a));
}
