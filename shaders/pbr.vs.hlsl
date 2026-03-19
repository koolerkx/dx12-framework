#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/instance_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"
#include "common_types.hlsli"

struct VSOUT {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float3 worldPos : TEXCOORD1;
  float3 worldTangent : TEXCOORD2;
  float3 worldBitangent : TEXCOORD3;
  float4 overlayColor : TEXCOORD4;
};

VSOUT main(VS_IN_PBR input, uint instanceID : SV_InstanceID) {
  float4x4 worldMat;
  float3x3 normalMat;
  float4 instColor;
  float4 overlayCol;
  float2 uvOffset;
  float2 uvScale;

  if (g_ObjectCB.flags & OBJECT_FLAG_INSTANCED) {
    InstanceData inst = g_InstanceBuffer[instanceID];
    worldMat = inst.world;
    normalMat = ComputeNormalMatrix(inst.world);
    instColor = inst.color;
    overlayCol = inst.overlayColor;
    uvOffset = inst.uvOffset;
    uvScale = inst.uvScale;
  } else {
    worldMat = g_ObjectCB.world;
    normalMat = (float3x3)g_ObjectCB.normalMatrix;
    instColor = float4(1, 1, 1, 1);
    overlayCol = float4(0, 0, 0, 0);
    uvOffset = g_ObjectCB.uvOffset;
    uvScale = g_ObjectCB.uvScale;
  }

  VSOUT output;
  float4 worldPos = mul(float4(input.position, 1.0f), worldMat);
  output.worldPos = worldPos.xyz;
  output.position = mul(worldPos, g_FrameCB.viewProj);

  output.worldNormal = normalize(mul(input.normal, normalMat));
  output.worldTangent = normalize(mul(input.tangent.xyz, normalMat));
  output.worldBitangent =
      cross(output.worldNormal, output.worldTangent) * input.tangent.w;

  output.uv = input.uv * uvScale + uvOffset;
  output.color = input.color * instColor;
  output.overlayColor = overlayCol;
  return output;
}
