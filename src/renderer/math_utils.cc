#include "renderer/math_utils.h"

#include <cmath>

// Based on approach from: https://learnopengl.com/Getting-started/Camera
// NOLINTBEGIN(bugprone-easily-swappable-parameters): Conventional shape
simd::float4x4 DirectedViewMatrix(const simd::float3& eye_position,
                                  const simd::float3& target_position,
                                  const simd::float3& world_up) {
  // NOLINTEND(bugprone-easily-swappable-parameters)
  const simd::float3 eye_forward =
      simd::normalize(target_position - eye_position);
  const simd::float3 eye_right =
      simd::normalize(simd::cross(eye_forward, world_up));
  const simd::float3 eye_up = simd::cross(eye_right, eye_forward);

  return simd::float4x4{
      simd::float4{eye_right.x, eye_up.x, -eye_forward.x, 0.0F},
      simd::float4{eye_right.y, eye_up.y, -eye_forward.y, 0.0F},
      simd::float4{eye_right.z, eye_up.z, -eye_forward.z, 0.0F},
      simd::float4{-simd::dot(eye_right, eye_position),
                   -simd::dot(eye_up, eye_position),
                   simd::dot(eye_forward, eye_position), 1.0F}};
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Conventional shape, again
simd::float4x4 PerspectiveProjectionMatrix(float fov_y_radians,
                                           float aspect_ratio,
                                           float near_z_distance,
                                           float far_z_distance) {
  // NOLINTEND(bugprone-easily-swappable-parameters)
  const float y_scale = 1.0F / std::tan(fov_y_radians / 2.0F);
  const float x_scale = y_scale / aspect_ratio;
  const float depth_scale = far_z_distance / (near_z_distance - far_z_distance);
  const float depth_offset =
      (near_z_distance * far_z_distance) / (near_z_distance - far_z_distance);

  return simd::float4x4{simd::float4{x_scale, 0.0F, 0.0F, 0.0F},
                        simd::float4{0.0F, y_scale, 0.0F, 0.0F},
                        simd::float4{0.0F, 0.0F, depth_scale, -1.0F},
                        simd::float4{0.0F, 0.0F, depth_offset, 0.0F}};
}

simd::float3x3 NormalMatrix(const simd::float4x4& model_matrix) {
  return simd::transpose(simd::inverse(simd::float3x3{
      model_matrix.columns[0].xyz,
      model_matrix.columns[1].xyz,
      model_matrix.columns[2].xyz,
  }));
}

simd::float4x4 TranslationMatrix(const simd::float3& offset) {
  return simd::float4x4{simd::float4{1.0F, 0.0F, 0.0F, 0.0F},
                        simd::float4{0.0F, 1.0F, 0.0F, 0.0F},
                        simd::float4{0.0F, 0.0F, 1.0F, 0.0F},
                        simd::float4{offset.x, offset.y, offset.z, 1.0F}};
}

// TODO(Blake): Look at GLM's implementation, it allows for a unit axis. See
// about applying the same approach here
simd::float4x4 XRotationMatrix(float angle_radians) {
  const float cos_angle = std::cos(angle_radians);
  const float sin_angle = std::sin(angle_radians);
  return simd::float4x4{simd::float4{1.0F, 0.0F, 0.0F, 0.0F},
                        simd::float4{0.0F, cos_angle, sin_angle, 0.0F},
                        simd::float4{0.0F, -sin_angle, cos_angle, 0.0F},
                        simd::float4{0.0F, 0.0F, 0.0F, 1.0F}};
}

simd::float4x4 YRotationMatrix(float angle_radians) {
  const float cos_angle = std::cos(angle_radians);
  const float sin_angle = std::sin(angle_radians);
  return simd::float4x4{simd::float4{cos_angle, 0.0F, -sin_angle, 0.0F},
                        simd::float4{0.0F, 1.0F, 0.0F, 0.0F},
                        simd::float4{sin_angle, 0.0F, cos_angle, 0.0F},
                        simd::float4{0.0F, 0.0F, 0.0F, 1.0F}};
}

simd::float4x4 ZRotationMatrix(float angle_radians) {
  const float cos_angle = std::cos(angle_radians);
  const float sin_angle = std::sin(angle_radians);
  return simd::float4x4{simd::float4{cos_angle, sin_angle, 0.0F, 0.0F},
                        simd::float4{-sin_angle, cos_angle, 0.0F, 0.0F},
                        simd::float4{0.0F, 0.0F, 1.0F, 0.0F},
                        simd::float4{0.0F, 0.0F, 0.0F, 1.0F}};
}
