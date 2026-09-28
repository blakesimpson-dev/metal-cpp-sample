
#ifndef METAL_CPP_SAMPLE_RENDERER_CAMERA_H_
#define METAL_CPP_SAMPLE_RENDERER_CAMERA_H_

#include <simd/simd.h>

class Camera {
 public:
  void SetLookAt(const simd::float3& camera_position,
                 const simd::float3& target_position);
  void SetPerspective(float fov_y_radians, float near_z_distance,
                      float far_z_distance);
  void SetAspectRatio(float aspect_ratio) { aspect_ratio_ = aspect_ratio; }
  [[nodiscard]]
  simd::float3 Position() const {
    return camera_position_;
  }
  [[nodiscard]]
  simd::float4x4 ViewMatrix() const;
  [[nodiscard]]
  simd::float4x4 ProjectionMatrix() const;

 private:
  simd::float3 camera_position_{};
  simd::float3 target_position_{};
  float fov_y_radians_ = 0.0F;
  float aspect_ratio_ = 1.0F;
  float near_z_distance_ = 0.0F;
  float far_z_distance_ = 0.0F;
};

#endif  // METAL_CPP_SAMPLE_RENDERER_CAMERA_H_
