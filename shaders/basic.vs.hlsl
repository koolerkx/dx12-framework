#include "basic_type.hlsli"

BasicType main(VSIN input) {
  BasicType output;
  output.position = mul(float4(input.pos, 1.0f), g_WorldViewProj);
  output.uv = input.uv;
  return output;
}