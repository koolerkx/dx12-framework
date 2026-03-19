#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/instance_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
};

struct VSOutput {
  float4 position : SV_POSITION;
  float3 viewNormal : TEXCOORD0;
  float viewDepth : TEXCOORD1;
};

VSOutput main(VSInput input, uint instanceID : SV_InstanceID) {
  float4x4 worldMat;
  float3x3 normalMat;

  if (g_ObjectCB.flags & OBJECT_FLAG_INSTANCED) {
    InstanceData inst = g_InstanceBuffer[instanceID];
    worldMat = inst.world;
    normalMat = ComputeNormalMatrix(inst.world);
  } else {
    worldMat = g_ObjectCB.world;
    normalMat = (float3x3)g_ObjectCB.normalMatrix;
  }

  VSOutput output;
  float4 worldPos = mul(float4(input.position, 1.0), worldMat);
  output.position = mul(worldPos, g_FrameCB.viewProj);

  float3 worldNormal = normalize(mul(input.normal, normalMat));
  output.viewNormal = normalize(mul(worldNormal, (float3x3)g_FrameCB.view));
  output.viewDepth = mul(worldPos, g_FrameCB.view).z;

  return output;
}
