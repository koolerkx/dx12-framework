#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/object_data.hlsli"
#include "common_types.hlsli"

struct VSOUT {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : BASE_COLOR;
  float3 worldPos : TEXCOORD1;
  float4 overlayColor : OVERLAY_COLOR;
  nointerpolation uint objectIndex : OBJECT_INDEX;
};

VSOUT main(VS_IN_3D input, uint objectIndex : OBJECT_INDEX) {
  ObjectData obj = g_ObjectBuffer[objectIndex];

  float4x4 worldMat = obj.world;
  float3x3 normalMat = ComputeNormalMatrix(obj.world);

  VSOUT output;
  float4 worldPos = mul(float4(input.position, 1.0f), worldMat);
  output.worldPos = worldPos.xyz;
  output.position = mul(worldPos, g_FrameCB.viewProj);
  output.worldNormal = normalize(mul(input.normal, normalMat));
  output.uv = input.uv * obj.uvScale + obj.uvOffset;
  output.color = input.color * obj.color;
  output.overlayColor = obj.overlayColor;
  output.objectIndex = objectIndex;
  return output;
}
