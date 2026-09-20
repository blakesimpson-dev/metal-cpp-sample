#include "renderer/math_utils.h"

#include <cmath>

// Based on approach from: https://learnopengl.com/Getting-started/Camera
// NOLINTBEGIN(bugprone-easily-swappable-parameters): Conventional shape
simd::float4x4 LookAtView(const simd::float3& eye_position,
                          const simd::float3& target_position,
                          const simd::float3& world_up) {
  // NOLINTEND(bugprone-easily-swappable-parameters)
  const simd::float3 eye_forward =
      simd::normalize(target_position - eye_position);
  const simd::float3 eye_right =
      simd::normalize(simd::cross(eye_forward, world_up));
  const simd::float3 eye_up = simd::cross(eye_right, eye_forward);

  const simd::float4x4 view_matrix(
      simd::float4{eye_right.x, eye_up.x, -eye_forward.x, 0.0F},
      simd::float4{eye_right.y, eye_up.y, -eye_forward.y, 0.0F},
      simd::float4{eye_right.z, eye_up.z, -eye_forward.z, 0.0F},
      simd::float4{-simd::dot(eye_right, eye_position),
                   -simd::dot(eye_up, eye_position),
                   simd::dot(eye_forward, eye_position), 1.0F});

  return view_matrix;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters): Conventional shape, again
simd::float4x4 PerspectiveProjection(float fov_y_radians, float aspect_ratio,
                                     float near_z_distance,
                                     float far_z_distance) {
  // NOLINTEND(bugprone-easily-swappable-parameters)
  const float y_scale = 1.0F / std::tan(fov_y_radians / 2.0F);
  const float x_scale = y_scale / aspect_ratio;
  const float depth_scale = far_z_distance / (near_z_distance - far_z_distance);
  const float depth_offset =
      (near_z_distance * far_z_distance) / (near_z_distance - far_z_distance);

  const simd::float4x4 projection_matrix(
      simd::float4{x_scale, 0.0F, 0.0F, 0.0F},
      simd::float4{0.0F, y_scale, 0.0F, 0.0F},
      simd::float4{0.0F, 0.0F, depth_scale, -1.0F},
      simd::float4{0.0F, 0.0F, depth_offset, 0.0F});

  return projection_matrix;
}
