#include "ConstantBuffer/object_cb.hlsli"
#include "common_types.hlsli"

struct VSOUT {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

VSOUT main(VS_IN_3D input) {
  VSOUT output;
  output.position = mul(float4(input.position, 1.0f), g_ObjectCB.worldViewProj);
  output.worldNormal = normalize(mul(input.normal, (float3x3)g_ObjectCB.world));
  output.uv = input.uv * g_ObjectCB.uvScale + g_ObjectCB.uvOffset;
  output.color = input.color;
  return output;
}
