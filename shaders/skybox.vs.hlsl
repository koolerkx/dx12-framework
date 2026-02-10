#include "ConstantBuffer/frame_cb.hlsli"

struct VSIN {
  float3 position : POSITION;
};

struct VSOUT {
  float4 position : SV_POSITION;
  float3 local_pos : TEXCOORD;
};

VSOUT main(VSIN input) {
  VSOUT output;

  float4x4 view_no_translate = g_FrameCB.view;
  view_no_translate[3] = float4(0, 0, 0, 1);

  float4 clip_pos = mul(float4(input.position, 1.0), mul(view_no_translate, g_FrameCB.proj));
  output.position = clip_pos.xyww;
  output.local_pos = input.position;

  return output;
}
