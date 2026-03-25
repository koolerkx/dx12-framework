#include "ConstantBuffer/custom_cb.hlsli"

struct NeonGridCB {
  float3 gridColor;
  float gridDivisions;
  float3 fillColor;
  float fillOpacity;
  float gridLineWidth;
  float glowIntensity;
};
ConstantBuffer<NeonGridCB> g_NeonGridCB : register(CUSTOM_CB_SLOT);

struct PSIN {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : BASE_COLOR;
  float3 worldPos : TEXCOORD1;
  float4 overlayColor : OVERLAY_COLOR;
};

float GridLine(float coord, float divisions, float lineWidth) {
  float cellCoord = frac(coord * divisions);
  float distToEdge = min(cellCoord, 1.0 - cellCoord);
  float halfWidth = lineWidth * 0.5;
  return smoothstep(halfWidth + halfWidth * 0.5, halfWidth, distToEdge);
}

float4 main(PSIN input) : SV_TARGET {
  float gridX = GridLine(input.uv.x, g_NeonGridCB.gridDivisions,
                         g_NeonGridCB.gridLineWidth);
  float gridY = GridLine(input.uv.y, g_NeonGridCB.gridDivisions,
                         g_NeonGridCB.gridLineWidth);
  float lineIntensity = saturate(gridX + gridY);

  float3 vertexColor = input.color.rgb;
  float3 blendedColor =
      1.0 - (1.0 - vertexColor) * (1.0 - g_NeonGridCB.gridColor);

  float3 gridColor = blendedColor * lineIntensity * g_NeonGridCB.glowIntensity;

  float fillMask = (1.0 - lineIntensity) * g_NeonGridCB.fillOpacity;
  float3 fillContrib = vertexColor * g_NeonGridCB.fillColor * fillMask;

  float3 finalColor = gridColor + fillContrib;
  float alpha = saturate(lineIntensity + fillMask);

  return float4(finalColor, alpha);
}
