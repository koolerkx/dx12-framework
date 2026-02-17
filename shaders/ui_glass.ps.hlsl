#include "ConstantBuffer/custom_cb.hlsli"
#include "ConstantBuffer/frame_cb.hlsli"


struct GlassCB {
  uint blurSrvIndex;
  float distortionStrength;
  float tintAlpha;
  float _pad;
  float4 tintColor;
};
ConstantBuffer<GlassCB> g_GlassCB : register(CUSTOM_CB_SLOT);

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

float4 main(PSIN input) : SV_TARGET {
  float2 screen_uv = input.position.xy / g_FrameCB.screenSize;

  float2 center_offset = input.uv - 0.5;
  float2 distortion = center_offset * g_GlassCB.distortionStrength;
  float2 sample_uv = saturate(screen_uv + distortion);

  Texture2D blur_tex = g_Textures[g_GlassCB.blurSrvIndex];
  float3 blurred =
      blur_tex.Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], sample_uv).rgb;

  float3 result = lerp(blurred, g_GlassCB.tintColor.rgb, g_GlassCB.tintAlpha);

  return float4(result, 1.0);
}
