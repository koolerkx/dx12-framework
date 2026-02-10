// ACES Filmic Tone Mapping Approximation by Krzysztof Narkowicz
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 ACESFilmic(float3 x) {
  const float a = 2.51f;
  const float b = 0.03f;
  const float c = 2.43f;
  const float d = 0.59f;
  const float e = 0.14f;
  return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Linear to sRGB conversion (manual conversion since swapchain is UNORM)
float3 LinearToSRGB(float3 color) {
  float3 srgb;
  for (int i = 0; i < 3; ++i) {
    if (color[i] <= 0.0031308f) {
      srgb[i] = color[i] * 12.92f;
    } else {
      srgb[i] = 1.055f * pow(color[i], 1.0f / 2.4f) - 0.055f;
    }
  }
  return saturate(srgb);
}

struct ToneMapCB {
  float exposure;
  uint debugMode;
  uint hdrSrvIndex;
  uint _padding;
};
ConstantBuffer<ToneMapCB> g_ToneMapCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  float3 hdr_color =
      g_Textures[g_ToneMapCB.hdrSrvIndex].Sample(g_Samplers[0], input.uv).rgb;

  if (g_ToneMapCB.debugMode == 1) {
    return float4(hdr_color / 16.0, 1.0);
  }

  float3 exposed = hdr_color * g_ToneMapCB.exposure;
  float3 linear_ldr = ACESFilmic(exposed);
  float3 srgb_ldr = LinearToSRGB(linear_ldr);

  return float4(srgb_ldr, 1.0);
}
