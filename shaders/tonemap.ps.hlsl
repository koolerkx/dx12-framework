#include "basic_type.hlsli"

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

cbuffer ToneMapCB : register(b2) {
  float g_Exposure;
  uint g_DebugMode;
  uint g_HdrSrvIndex;
  uint _padding;
}

// g_Textures is declared in basic_type.hlsli, but g_Samplers[] needs to be declared here
SamplerState g_Samplers[] : register(s0, space0);

float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET {
  // Sample HDR render target using index from constant buffer
  float3 hdr_color = g_Textures[g_HdrSrvIndex].Sample(g_Samplers[0], uv).rgb;

  // Debug view: skip tone mapping
  if (g_DebugMode == 1) {
    return float4(hdr_color / 16.0, 1.0);
  }

  // Apply exposure
  float3 exposed = hdr_color * g_Exposure;

  // Tone map to LINEAR LDR
  float3 linear_ldr = ACESFilmic(exposed);

  // Convert linear to sRGB (manual since swapchain is UNORM format)
  float3 srgb_ldr = LinearToSRGB(linear_ldr);

  // Output alpha = 1.0 (opaque swapchain)
  return float4(srgb_ldr, 1.0);
}
