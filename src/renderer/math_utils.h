#pragma once

#include <simd/simd.h>

simd::float4x4 LookAtView(const simd::float3& eye_position,
                          const simd::float3& target_position,
                          const simd::float3& world_up);

simd::float4x4 PerspectiveProjection(float fov_y_radians, float aspect_ratio,
                                     float near_z_distance,
                                     float far_z_distance);
