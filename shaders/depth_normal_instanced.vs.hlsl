#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/instance_cb.hlsli"

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  uint instanceID : SV_InstanceID;
};

struct VSOutput {
  float4 position : SV_POSITION;
  float3 viewNormal : TEXCOORD0;
  float viewDepth : TEXCOORD1;
};

VSOutput main(VSInput input) {
  InstanceData inst = g_InstanceBuffer[input.instanceID];

  VSOutput output;
  float4 worldPos = mul(float4(input.position, 1.0), inst.world);
  output.position = mul(worldPos, g_FrameCB.viewProj);

  float3 worldNormal =
      normalize(mul(input.normal, (float3x3)inst.normalMatrix));
  output.viewNormal = normalize(mul(worldNormal, (float3x3)g_FrameCB.view));
  output.viewDepth = mul(worldPos, g_FrameCB.view).z;

  return output;
}
