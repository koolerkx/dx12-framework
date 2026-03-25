#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/object_data.hlsli"
#include "common_types.hlsli"

struct VSOUT {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : BASE_COLOR;
  float4 overlayColor : OVERLAY_COLOR;
  nointerpolation uint objectIndex : OBJECT_INDEX;
};

VSOUT main(VSIN input, uint objectIndex : OBJECT_INDEX) {
  ObjectData obj = g_ObjectBuffer[objectIndex];

  VSOUT output;
  float4 worldPos = mul(float4(input.position, 1.0f), obj.world);
  output.position = mul(worldPos, g_FrameCB.viewProj);
  output.uv = input.uv * obj.uvScale + obj.uvOffset;
  output.color = input.color * obj.color;
  output.overlayColor = obj.overlayColor;
  output.objectIndex = objectIndex;
  return output;
}
