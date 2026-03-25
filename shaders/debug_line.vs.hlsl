#include "ConstantBuffer/frame_cb.hlsli"

struct VSIN {
  float3 position : POSITION;
  float4 color : COLOR;
};

struct VSOUT {
  float4 position : SV_POSITION;
  float4 color : BASE_COLOR;
};

VSOUT main(VSIN input) {
  VSOUT output;
  output.position = mul(float4(input.position, 1.0f), g_FrameCB.viewProj);
  output.color = input.color;
  return output;
}
