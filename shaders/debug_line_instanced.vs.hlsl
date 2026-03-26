#include "ConstantBuffer/frame_cb.hlsli"

struct LineInstance {
  float3 position;
  float3 axis_x;
  float3 axis_y;
  float3 axis_z;
  float4 color;
};

StructuredBuffer<LineInstance> g_Instances : register(t0, space3);

struct VSIN {
  float3 position : POSITION;
  float4 vertex_color : COLOR;
  uint instanceIndex : OBJECT_INDEX;
};

struct VSOUT {
  float4 position : SV_POSITION;
  float4 color : BASE_COLOR;
};

VSOUT main(VSIN input) {
  VSOUT output;
  LineInstance inst = g_Instances[input.instanceIndex];
  float3 world_pos = inst.position + input.position.x * inst.axis_x +
                     input.position.y * inst.axis_y +
                     input.position.z * inst.axis_z;
  output.position = mul(float4(world_pos, 1.0f), g_FrameCB.viewProj);
  output.color = inst.color;
  return output;
}
