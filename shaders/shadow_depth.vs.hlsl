#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/object_data.hlsli"

struct SHADOW_VSIN {
  float3 position : POSITION;
  uint objectIndex : OBJECT_INDEX;
};

float4 main(SHADOW_VSIN input) : SV_POSITION {
  ObjectData obj = g_ObjectBuffer[input.objectIndex];
  float4 worldPos = mul(float4(input.position, 1.0f), obj.world);
  return mul(worldPos, g_FrameCB.viewProj);
}
