struct SSAOBlurCB {
  uint aoSrvIndex;
  uint normalDepthSrvIndex;
  float2 texelSize;
};
ConstantBuffer<SSAOBlurCB> g_BlurCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

static const float DEPTH_THRESHOLD_RATIO = 0.005;

float4 main(PSIN input) : SV_TARGET {
  float4 centerND = g_Textures[g_BlurCB.normalDepthSrvIndex]
                        .Sample(g_Samplers[4], input.uv);
  float centerDepth = centerND.w;
  float3 centerNormal = normalize(centerND.xyz);
  float depthThreshold = max(centerDepth * DEPTH_THRESHOLD_RATIO, 0.01);

  float totalAO = 0.0;
  float totalWeight = 0.0;

  [unroll] for (int x = -2; x <= 2; ++x) {
    [unroll] for (int y = -2; y <= 2; ++y) {
      float2 offset = float2(x, y) * g_BlurCB.texelSize;
      float2 sampleUV = input.uv + offset;

      float sampleAO =
          g_Textures[g_BlurCB.aoSrvIndex].Sample(g_Samplers[4], sampleUV).r;
      float4 sampleND = g_Textures[g_BlurCB.normalDepthSrvIndex]
                             .Sample(g_Samplers[4], sampleUV);

      float depthWeight =
          saturate(1.0 - abs(centerDepth - sampleND.w) / depthThreshold);
      float normalDot = saturate(dot(centerNormal, normalize(sampleND.xyz)));
      float weight = depthWeight * normalDot * normalDot;

      totalAO += sampleAO * weight;
      totalWeight += weight;
    }
  }

  return totalAO / max(totalWeight, 1.0);
}
