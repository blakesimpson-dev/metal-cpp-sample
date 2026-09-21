#include "renderer/camera.h"

#include "renderer/math_utils.h"

namespace {
const simd::float3 kWorldUp{0.0F, 1.0F, 0.0F};
}  // namespace

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): Conventional shape.
void Camera::SetLookAt(const simd::float3& camera_position,
                       const simd::float3& target_position) {
  camera_position_ = camera_position;
  target_position_ = target_position;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): Conventional shape.
void Camera::SetPerspective(float fov_y_radians, float near_z_distance,
                            float far_z_distance) {
  fov_y_radians_ = fov_y_radians;
  near_z_distance_ = near_z_distance;
  far_z_distance_ = far_z_distance;
}

simd::float4x4 Camera::ViewMatrix() const {
  return MakeViewMatrix(camera_position_, target_position_, kWorldUp);
}

simd::float4x4 Camera::ProjectionMatrix() const {
  return MakeProjectionMatrix(fov_y_radians_, aspect_ratio_, near_z_distance_,
                              far_z_distance_);
}
