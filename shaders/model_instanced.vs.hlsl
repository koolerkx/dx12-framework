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
};

VSOUT main(VS_IN_PBR input, uint instanceID : SV_InstanceID) {
  InstanceData inst = g_InstanceBuffer[instanceID];

  VSOUT output;
  float4 worldPos = mul(float4(input.position, 1.0f), inst.world);
  output.worldPos = worldPos.xyz;
  output.position = mul(worldPos, g_FrameCB.viewProj);

  float3x3 normalMat = (float3x3)inst.normalMatrix;
  output.worldNormal = normalize(mul(input.normal, normalMat));
  output.worldTangent = normalize(mul(input.tangent.xyz, normalMat));
  output.worldBitangent =
      cross(output.worldNormal, output.worldTangent) * input.tangent.w;

  output.uv = input.uv * g_ObjectCB.uvScale + g_ObjectCB.uvOffset;
  output.color = input.color * inst.color;
  return output;
}
