#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/instance_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"
#include "common_types.hlsli"

struct VSOUT {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : BASE_COLOR;
  float3 worldPos : TEXCOORD1;
  float4 overlayColor : OVERLAY_COLOR;
};

VSOUT main(VS_IN_3D input, uint instanceID : SV_InstanceID) {
  float4x4 worldMat;
  float3x3 normalMat;
  float4 instColor;
  float2 uvOffset;
  float2 uvScale;
  float4 overlayCol;

  if (g_ObjectCB.flags & OBJECT_FLAG_INSTANCED) {
    InstanceData inst = g_InstanceBuffer[instanceID];
    worldMat = inst.world;
    normalMat = ComputeNormalMatrix(inst.world);
    instColor = inst.color;
    uvOffset = inst.uvOffset;
    uvScale = inst.uvScale;
    overlayCol = inst.overlayColor;
  } else {
    worldMat = g_ObjectCB.world;
    normalMat = (float3x3)g_ObjectCB.normalMatrix;
    instColor = float4(1, 1, 1, 1);
    uvOffset = g_ObjectCB.uvOffset;
    uvScale = g_ObjectCB.uvScale;
    overlayCol = float4(0, 0, 0, 0);
  }

  VSOUT output;
  float4 worldPos = mul(float4(input.position, 1.0f), worldMat);
  output.worldPos = worldPos.xyz;
  output.position = mul(worldPos, g_FrameCB.viewProj);
  output.worldNormal = normalize(mul(input.normal, normalMat));
  output.uv = input.uv * uvScale + uvOffset;
  output.color = input.color * instColor;
  output.overlayColor = overlayCol;
  return output;
}
