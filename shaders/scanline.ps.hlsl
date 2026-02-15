#include "ConstantBuffer/custom_cb.hlsli"
#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"

struct ScanlineCB {
  float3 primaryColor;
  float gridDivisions;
  float3 secondaryColor;
  float gridLineWidth;
  float scanlineSpeed;
  float scanlineWidth;
  float glowIntensity;
  float edgeGlowWidth;
};
ConstantBuffer<ScanlineCB> g_ScanlineCB : register(CUSTOM_CB_SLOT);

struct PSIN {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float3 worldPos : TEXCOORD1;
};

float GridLine(float coord, float divisions, float lineWidth) {
  float cellCoord = frac(coord * divisions);
  float distToEdge = min(cellCoord, 1.0 - cellCoord);
  float halfWidth = lineWidth * 0.5;
  return smoothstep(halfWidth + halfWidth * 0.5, halfWidth, distToEdge);
}

float4 main(PSIN input) : SV_TARGET {
  float3 wp = input.worldPos;

  float3 absNormal = abs(input.worldNormal);
  float2 gridUV;
  if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
    gridUV = wp.xz;
  } else if (absNormal.x > absNormal.z) {
    gridUV = wp.yz;
  } else {
    gridUV = wp.xy;
  }

  float gridX = GridLine(gridUV.x, g_ScanlineCB.gridDivisions,
                         g_ScanlineCB.gridLineWidth);
  float gridY = GridLine(gridUV.y, g_ScanlineCB.gridDivisions,
                         g_ScanlineCB.gridLineWidth);
  float grid = saturate(gridX + gridY);

  float scanY = frac(wp.y * 0.5 - g_FrameCB.time * g_ScanlineCB.scanlineSpeed);
  float scanline =
      smoothstep(g_ScanlineCB.scanlineWidth, 0.0, abs(scanY - 0.5));

  float lineIntensity = saturate(grid);
  float3 gridColor =
      g_ScanlineCB.primaryColor * lineIntensity * g_ScanlineCB.glowIntensity;
  float3 scanColor = g_ScanlineCB.secondaryColor * scanline * 0.8;

  float3 finalColor = gridColor + scanColor;
  float alpha = saturate(lineIntensity + scanline * 0.5);

  return float4(finalColor, alpha);
}
