#pragma once

#include <simd/simd.h>

simd::float4x4 DirectedViewMatrix(const simd::float3& eye_position,
                                  const simd::float3& target_position,
                                  const simd::float3& world_up);

simd::float4x4 PerspectiveProjectionMatrix(float fov_y_radians,
                                           float aspect_ratio,
                                           float near_z_distance,
                                           float far_z_distance);

simd::float3x3 NormalMatrix(const simd::float4x4& model_matrix);

simd::float4x4 TranslationMatrix(const simd::float3& offset);

simd::float4x4 XRotationMatrix(float angle_radians);

simd::float4x4 YRotationMatrix(float angle_radians);

simd::float4x4 ZRotationMatrix(float angle_radians);
