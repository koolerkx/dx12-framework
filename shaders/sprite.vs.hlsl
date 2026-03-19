#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/instance_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"
#include "common_types.hlsli"

struct VSOUT {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : BASE_COLOR;
  float4 overlayColor : OVERLAY_COLOR;
};

VSOUT main(VSIN input, uint instanceID : SV_InstanceID) {
  VSOUT output;

  if (g_ObjectCB.flags & OBJECT_FLAG_INSTANCED) {
    InstanceData inst = g_InstanceBuffer[instanceID];
    float4 worldPos = mul(float4(input.position, 1.0f), inst.world);
    output.position = mul(worldPos, g_FrameCB.viewProj);
    output.uv = input.uv * inst.uvScale + inst.uvOffset;
    output.color = input.color * inst.color;
    output.overlayColor = inst.overlayColor;
  } else {
    output.position = mul(float4(input.position, 1.0f), g_ObjectCB.worldViewProj);
    output.uv = input.uv * g_ObjectCB.uvScale + g_ObjectCB.uvOffset;
    output.color = input.color;
    output.overlayColor = float4(0, 0, 0, 0);
  }

  return output;
}
