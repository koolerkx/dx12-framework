#include "ConstantBuffer/custom_cb.hlsli"
#include "ConstantBuffer/frame_cb.hlsli"


struct GlassCB {
  uint blurSrvIndex;
  float distortionStrength;
  float tintAlpha;
  float chromaticStrength;
  float4 tintColor;
  float fresnelPower;
  float fresnelIntensity;
  float specularIntensity;
  float specularSharpness;
  float specularOffsetX;
  float specularOffsetY;
  float edgeShadowStrength;
  float panelAspect;
  float darken;
  float3 _pad;
};
ConstantBuffer<GlassCB> g_GlassCB : register(CUSTOM_CB_SLOT);

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

static const float SQUIRCLE_POWER = 8.0;
static const float CORNER_RADIUS = 0.3;

float ComputeEdgeFactor(float2 uv_centered, float panel_aspect) {
  float2 scale = (panel_aspect >= 1.0)
      ? float2(panel_aspect, 1.0)
      : float2(1.0, 1.0 / panel_aspect);
  float2 pos = abs(uv_centered) * 2.0 * scale;
  float2 q = max(pos - (scale - CORNER_RADIUS), 0.0) / CORNER_RADIUS;
  return pow(q.x, SQUIRCLE_POWER) + pow(q.y, SQUIRCLE_POWER);
}

float4 main(PSIN input) : SV_TARGET {
  float2 screen_uv = input.position.xy / g_FrameCB.screenSize;
  float2 uv_centered = input.uv - 0.5;

  float edge = saturate(ComputeEdgeFactor(uv_centered, g_GlassCB.panelAspect));

  float2 dist_dir = normalize(uv_centered + 0.0001);
  float2 distortion = dist_dir * g_GlassCB.distortionStrength * smoothstep(0.3, 1.0, edge);
  float2 sample_uv = screen_uv + distortion;

  Texture2D blur_tex = g_Textures[g_GlassCB.blurSrvIndex];

  float3 blurred;
  if (g_GlassCB.chromaticStrength > 0.0) {
    float2 ca_offset = distortion * g_GlassCB.chromaticStrength * 20.0;
    blurred.r = blur_tex.Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], saturate(sample_uv + ca_offset)).r;
    blurred.g = blur_tex.Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], saturate(sample_uv)).g;
    blurred.b = blur_tex.Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], saturate(sample_uv - ca_offset)).b;
  } else {
    blurred = blur_tex.Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], saturate(sample_uv)).rgb;
  }

  float3 result = lerp(blurred, g_GlassCB.tintColor.rgb, g_GlassCB.tintAlpha);

  float fresnel = pow(saturate(edge), g_GlassCB.fresnelPower) * g_GlassCB.fresnelIntensity;
  result += fresnel;

  float2 spec_pos = uv_centered - float2(g_GlassCB.specularOffsetX, g_GlassCB.specularOffsetY);
  float specular = exp(-dot(spec_pos, spec_pos) * g_GlassCB.specularSharpness) * g_GlassCB.specularIntensity;
  result += specular;

  float shadow = smoothstep(0.85, 1.0, edge) * g_GlassCB.edgeShadowStrength;
  result *= (1.0 - shadow);

  result *= (1.0 - g_GlassCB.darken);

  return float4(result, 1.0);
}
