#include "ConstantBuffer/custom_cb.hlsli"
#include "ConstantBuffer/frame_cb.hlsli"

struct LaserBeamCB {
  float3 laserColor;
  float emissiveIntensity;
  float pulseSpeed;
  float pulseFrequency;
  float beamWidth;
  float endFadeRatio;
  float endFadePower;
  float3 _pad;
};
ConstantBuffer<LaserBeamCB> g_LaserCB : register(CUSTOM_CB_SLOT);

struct PSIN {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float3 worldPos : TEXCOORD1;
  float4 overlayColor : TEXCOORD2;
};

float4 main(PSIN input) : SV_TARGET {
  float3 viewDir = normalize(g_FrameCB.cameraPos - input.worldPos);
  float radialFade = saturate(abs(dot(normalize(input.worldNormal), viewDir)));
  radialFade = radialFade * radialFade;

  float radialDist = 1.0 - radialFade;
  float edgeWidth = fwidth(radialDist) * g_LaserCB.beamWidth;
  float edgeFade = smoothstep(1.0, 1.0 - edgeWidth, radialDist);

  float pulse = sin(input.uv.x * g_LaserCB.pulseFrequency - g_FrameCB.time * g_LaserCB.pulseSpeed);
  float pulseBand = pulse * 0.3 + 0.7;

  float fade = g_LaserCB.endFadeRatio;
  float lengthFade = smoothstep(0.0, fade, input.uv.x) * smoothstep(1.0, 1.0 - fade, input.uv.x);
  lengthFade = pow(lengthFade, g_LaserCB.endFadePower);

  float intensity = edgeFade * pulseBand * lengthFade;
  float3 finalColor = g_LaserCB.laserColor * g_LaserCB.emissiveIntensity * intensity;

  return float4(finalColor, intensity);
}
