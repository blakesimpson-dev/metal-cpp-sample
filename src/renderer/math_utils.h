#pragma once

#include <simd/simd.h>

simd::float4x4 MakeViewMatrix(const simd::float3& camera_position,
                              const simd::float3& target_position,
                              const simd::float3& world_up);

simd::float4x4 MakeProjectionMatrix(float fov_y_radians, float aspect_ratio,
                                    float near_z_distance,
                                    float far_z_distance);

simd::float3x3 MakeNormalMatrix(const simd::float4x4& model_matrix);

simd::float4x4 MakeTranslationMatrix(const simd::float3& offset);

simd::float4x4 MakeXRotationMatrix(float angle_radians);

simd::float4x4 MakeYRotationMatrix(float angle_radians);

simd::float4x4 MakeZRotationMatrix(float angle_radians);
