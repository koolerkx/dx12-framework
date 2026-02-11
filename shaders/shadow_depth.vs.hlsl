#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"

struct SHADOW_VSIN {
  float3 position : POSITION;
};

float4 main(SHADOW_VSIN input) : SV_POSITION {
  float4 worldPos = mul(float4(input.position, 1.0f), g_ObjectCB.world);
  return mul(worldPos, g_FrameCB.viewProj);
}
