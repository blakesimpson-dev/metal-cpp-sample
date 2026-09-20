#include <metal_stdlib>
using namespace metal;

#include "shader_types.h"

struct VertexOut {
  float4 position [[position]];
  half3 color;
};

VertexOut vertex VertexMain(uint vertex_id [[vertex_id]],
                            device const float3* positions
                            [[buffer(kBufferIndexPositions)]],
                            device const float3* colors
                            [[buffer(kBufferIndexColors)]]) {
  VertexOut out;
  out.position = float4(positions[vertex_id], 1.0);
  out.color = half3(colors[vertex_id]);
  return out;
}

half4 fragment FragmentMain(VertexOut stage_in [[stage_in]]) {
  return half4(stage_in.color, 1.0);
}
