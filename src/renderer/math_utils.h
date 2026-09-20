#pragma once

#include <simd/simd.h>

simd::float4x4 UpdateViewMatrix(const simd::float3& eye_position,
                                const simd::float3& target_position,
                                const simd::float3& world_up);

simd::float4x4 UpdateProjectionMatrix(float fov_y_radians, float aspect_ratio,
                                      float near_z_distance,
                                      float far_z_distance);

simd::float4x4 UpdateTranslationMatrix(const simd::float3& offset);

simd::float4x4 UpdateXAxisRotationMatrix(float angle_radians);

simd::float4x4 UpdateYAxisRotationMatrix(float angle_radians);

simd::float4x4 UpdateZAxisRotationMatrix(float angle_radians);
