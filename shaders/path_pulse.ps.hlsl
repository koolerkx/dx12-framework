#include "ConstantBuffer/custom_cb.hlsli"
#include "ConstantBuffer/frame_cb.hlsli"

struct PathPulseCB {
  float3 pulseColor;
  float emissiveIntensity;
  float pulseSpeed;
  float pulseFrequency;
  float pulseWidth;
  float distanceOffset;
  float segmentLength;
  float3 _pad;
};
ConstantBuffer<PathPulseCB> g_PulseCB : register(CUSTOM_CB_SLOT);

struct PSIN {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : BASE_COLOR;
  float3 worldPos : TEXCOORD1;
  float4 overlayColor : OVERLAY_COLOR;
};

float4 main(PSIN input) : SV_TARGET {
  float3 viewDir = normalize(g_FrameCB.cameraPos - input.worldPos);
  float radialFade = saturate(abs(dot(normalize(input.worldNormal), viewDir)));
  radialFade = radialFade * radialFade;

  float worldDist = input.uv.x * g_PulseCB.segmentLength + g_PulseCB.distanceOffset;
  float flow = worldDist * g_PulseCB.pulseFrequency - g_FrameCB.time * g_PulseCB.pulseSpeed;
  float pulse = smoothstep(1.0 - g_PulseCB.pulseWidth, 1.0, frac(flow));

  float intensity = pulse * radialFade;
  float3 finalColor = g_PulseCB.pulseColor * g_PulseCB.emissiveIntensity * intensity;

  return float4(finalColor, intensity);
}
