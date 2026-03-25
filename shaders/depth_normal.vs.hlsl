#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/object_data.hlsli"

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  uint objectIndex : OBJECT_INDEX;
};

struct VSOutput {
  float4 position : SV_POSITION;
  float3 viewNormal : TEXCOORD0;
  float viewDepth : TEXCOORD1;
};

VSOutput main(VSInput input) {
  ObjectData obj = g_ObjectBuffer[input.objectIndex];

  float4x4 worldMat = obj.world;
  float3x3 normalMat = ComputeNormalMatrix(obj.world);

  VSOutput output;
  float4 worldPos = mul(float4(input.position, 1.0), worldMat);
  output.position = mul(worldPos, g_FrameCB.viewProj);

  float3 worldNormal = normalize(mul(input.normal, normalMat));
  output.viewNormal = normalize(mul(worldNormal, (float3x3)g_FrameCB.view));
  output.viewDepth = mul(worldPos, g_FrameCB.view).z;

  return output;
}
