#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/instance_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"

struct SHADOW_VSIN {
  float3 position : POSITION;
};

float4 main(SHADOW_VSIN input, uint instanceID : SV_InstanceID) : SV_POSITION {
  float4x4 worldMat = (g_ObjectCB.flags & OBJECT_FLAG_INSTANCED)
                          ? g_InstanceBuffer[instanceID].world
                          : g_ObjectCB.world;

  float4 worldPos = mul(float4(input.position, 1.0f), worldMat);
  return mul(worldPos, g_FrameCB.viewProj);
}
