#include "ConstantBuffer/frame_cb.hlsli"

struct VSIN {
  float3 position : POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;

  float4 instanceWorld0 : INSTANCE_WORLD0;
  float4 instanceWorld1 : INSTANCE_WORLD1;
  float4 instanceWorld2 : INSTANCE_WORLD2;
  float4 instanceWorld3 : INSTANCE_WORLD3;
  float4 instanceColor : INSTANCE_COLOR;
  float2 instanceUVOffset : INSTANCE_UV_OFFSET;
  float2 instanceUVScale : INSTANCE_UV_SCALE;
};

struct VSOUT {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

VSOUT main(VSIN input) {
  VSOUT output;

  float4x4 instanceWorld = float4x4(input.instanceWorld0, input.instanceWorld1,
                                    input.instanceWorld2, input.instanceWorld3);

  float4 worldPos = mul(float4(input.position, 1.0f), instanceWorld);
  output.position = mul(worldPos, g_FrameCB.viewProj);

  output.uv = input.uv * input.instanceUVScale + input.instanceUVOffset;
  output.color = input.color * input.instanceColor;

  return output;
}
