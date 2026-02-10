#include "ConstantBuffer/object_cb.hlsli"
#include "common_types.hlsli"

struct VSOUT {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

VSOUT main(VSIN input) {
  VSOUT output;
  output.position = mul(float4(input.position, 1.0f), g_ObjectCB.worldViewProj);
  output.uv = input.uv * g_ObjectCB.uvScale + g_ObjectCB.uvOffset;
  output.color = input.color;
  return output;
}
