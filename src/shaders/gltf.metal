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

// Hash adapted from David Hoskin's 'Hash without sine'
// see: https://www.shadertoy.com/view/4djSRW
float Hash(float3 lattice_point) {
  float3 scaled = fract(lattice_point * 0.1031F);
  scaled += dot(scaled, scaled.zyx + 31.32F);
  return fract((scaled.x + scaled.y) * scaled.z);
}

// Value noise (hashed lattice values with cubic interpolation) adapted from
// 'The Book of Shaders, Noise'
// see: https://thebookofshaders.com/11
float ValueNoise(float3 position) {
  const float3 cell = floor(position);
  const float3 offset = fract(position);
  const float3 fade = offset * offset * (3.0F - 2.0F * offset);

  const float c000 = Hash(cell);
  const float c100 = Hash(cell + float3(1.0F, 0.0F, 0.0F));
  const float c010 = Hash(cell + float3(0.0F, 1.0F, 0.0F));
  const float c110 = Hash(cell + float3(1.0F, 1.0F, 0.0F));
  const float c001 = Hash(cell + float3(0.0F, 0.0F, 1.0F));
  const float c101 = Hash(cell + float3(1.0F, 0.0F, 1.0F));
  const float c011 = Hash(cell + float3(0.0F, 1.0F, 1.0F));
  const float c111 = Hash(cell + float3(1.0F, 1.0F, 1.0F));

  const float x00 = mix(c000, c100, fade.x);
  const float x10 = mix(c010, c110, fade.x);
  const float x01 = mix(c001, c101, fade.x);
  const float x11 = mix(c011, c111, fade.x);
  return mix(mix(x00, x10, fade.y), mix(x01, x11, fade.y), fade.z);
}

// Approach for perturb adapted from paper by Morten S. Mikkelsen
// see: https://mmikk.github.io/papers3d/mm_sfgrad_bump.pdf
float3 PerturbNormal(float3 surface_position, float3 surface_normal,
                     float2 height_derivatives) {
  const float3 sigma_x = normalize(dfdx(surface_position));
  const float3 sigma_y = normalize(dfdy(surface_position));
  const float3 r1 = cross(sigma_y, surface_normal);
  const float3 r2 = cross(surface_normal, sigma_x);
  const float determinant = dot(sigma_x, r1);
  const float3 surface_gradient =
      sign(determinant) *
      (height_derivatives.x * r1 + height_derivatives.y * r2);
  return normalize(abs(determinant) * surface_normal - surface_gradient);
}

half4 fragment GltfFragmentMain(VertexOut in [[stage_in]],
                                constant FragmentUniforms& material
                                [[buffer(kBufferIndexFragmentUniforms)]]) {
  const float3 noise_position = in.model_position * material.bump_frequency;
  const float height = ValueNoise(noise_position);
  const float2 height_derivatives =
      material.bump_strength *
      float2(ValueNoise(noise_position + dfdx(noise_position)) - height,
             ValueNoise(noise_position + dfdy(noise_position)) - height);
  const float3 normal = PerturbNormal(
      in.world_position, normalize(in.world_normal), height_derivatives);

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
