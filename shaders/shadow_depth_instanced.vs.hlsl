#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/instance_cb.hlsli"

struct SHADOW_VSIN {
  float3 position : POSITION;
  uint instanceID : SV_InstanceID;
};

float4 main(SHADOW_VSIN input) : SV_POSITION {
  InstanceData inst = g_InstanceBuffer[input.instanceID];
  float4 worldPos = mul(float4(input.position, 1.0f), inst.world);
  return mul(worldPos, g_FrameCB.viewProj);
}
