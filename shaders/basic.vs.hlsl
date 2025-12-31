#include "basic_type.hlsli"

BasicType main(VSIN input) {
  BasicType output;
  output.position = mul(float4(input.pos, 1.0f), g_WorldViewProj);
  // Apply UV transform for atlas/sprite sheet support
  output.uv = input.uv * g_UVScale + g_UVOffset;
  return output;
}