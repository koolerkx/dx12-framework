#include "ConstantBuffer/custom_cb.hlsli"
#include "ConstantBuffer/frame_cb.hlsli"

static const float TWO_PI = 6.28318530718;
static const float PI = 3.14159265359;

struct RadarRangeCB {
  float3 radarColor;
  float scanSpeed;
  float ringCount;
  float opacity;
  float emissiveIntensity;
  float ringWidth;
};
ConstantBuffer<RadarRangeCB> g_RadarCB : register(CUSTOM_CB_SLOT);

struct PSIN {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : BASE_COLOR;
  float3 worldPos : TEXCOORD1;
  float4 overlayColor : OVERLAY_COLOR;
};

float4 main(PSIN input) : SV_TARGET {
  float2 centered = input.uv * 2.0 - 1.0;
  float dist = length(centered);

  float discMask = 1.0 - smoothstep(0.93, 0.95, dist);

  float ringPhase = dist * g_RadarCB.ringCount * PI;
  float pixelWidth = fwidth(ringPhase);
  float halfWidth = g_RadarCB.ringWidth * pixelWidth * 0.5;
  float rings = sin(ringPhase);
  float ringLines = smoothstep(-halfWidth, halfWidth, rings) - smoothstep(halfWidth, halfWidth * 3.0, rings);

  float edgePixelWidth = fwidth(dist);
  float edgeHalfWidth = g_RadarCB.ringWidth * edgePixelWidth * 0.5;
  float edgeDist = abs(dist - 0.95);
  float edgeRing = 1.0 - smoothstep(0.0, edgeHalfWidth, edgeDist);

  float angle = atan2(centered.y, centered.x);
  float sweepAngle = frac(-g_FrameCB.time * g_RadarCB.scanSpeed) * TWO_PI;
  float angleDelta = fmod(angle - sweepAngle + TWO_PI * 3.0, TWO_PI);
  float sweepTrail = saturate(1.0 - angleDelta / (PI * 0.5));
  float sweepMask = sweepTrail * sweepTrail * 0.6;

  float centerGlow = exp(-dist * 8.0) * 0.3;

  float fillIntensity = (ringLines + sweepMask + centerGlow) * discMask;
  float intensity = max(fillIntensity, edgeRing);
  float3 finalColor = g_RadarCB.radarColor * intensity * g_RadarCB.emissiveIntensity;
  float alpha = intensity * g_RadarCB.opacity;

  return float4(finalColor, alpha);
}
