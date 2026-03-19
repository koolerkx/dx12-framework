struct SSAOBlurCB {
  uint aoSrvIndex;
  uint normalDepthSrvIndex;
  float2 texelSize;
};
ConstantBuffer<SSAOBlurCB> g_BlurCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

static const float DEPTH_THRESHOLD_RATIO = 0.005;

float4 main(PSIN input) : SV_TARGET {
  float4 centerND =
      g_Textures[g_BlurCB.normalDepthSrvIndex].Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], input.uv);
  float centerDepth = centerND.w;

  if (centerDepth <= 0.0 || dot(centerND.xyz, centerND.xyz) < 0.001) {
    return g_Textures[g_BlurCB.aoSrvIndex].Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], input.uv).r;
  }

  float3 centerNormal = normalize(centerND.xyz);
  float depthThreshold = max(centerDepth * DEPTH_THRESHOLD_RATIO, 0.01);

  float totalAO = 0.0;
  float totalWeight = 0.0;

  [unroll] for (int i = -2; i <= 2; ++i) {
    float2 offset = float2(i, i) * g_BlurCB.texelSize;
    float2 sampleUV = input.uv + offset;

    float sampleAO =
        g_Textures[g_BlurCB.aoSrvIndex].Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], sampleUV).r;
    float4 sampleND = g_Textures[g_BlurCB.normalDepthSrvIndex].Sample(
        g_Samplers[SAMPLER_LINEAR_CLAMP], sampleUV);

    if (dot(sampleND.xyz, sampleND.xyz) < 0.001) continue;

    float depthWeight =
        saturate(1.0 - abs(centerDepth - sampleND.w) / depthThreshold);
    float normalDot = saturate(dot(centerNormal, normalize(sampleND.xyz)));
    float weight = depthWeight * normalDot * normalDot;

    totalAO += sampleAO * weight;
    totalWeight += weight;
  }

  return totalAO / max(totalWeight, 1.0);
}
