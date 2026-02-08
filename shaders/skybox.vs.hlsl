#include "basic_type.hlsli"

struct SkyboxVSInput {
  float3 pos : POSITION;
};

struct SkyboxVSOutput {
  float4 position : SV_POSITION;
  float3 local_pos : TEXCOORD0;
};

SkyboxVSOutput main(SkyboxVSInput input) {
  SkyboxVSOutput output;

  float4x4 view_no_translate = g_View;
  view_no_translate[3] = float4(0, 0, 0, 1);

  float4 clip_pos = mul(float4(input.pos, 1.0), mul(view_no_translate, g_Proj));
  output.position = clip_pos.xyww;
  output.local_pos = input.pos;

  return output;
}
