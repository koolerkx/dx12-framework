#include "basic_type.hlsli"

struct VSOutput {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

VSOutput main(uint vertexID : SV_VertexID) {
  VSOutput output;

  output.uv = float2((vertexID == 2) ? 2.0 : 0.0, (vertexID == 1) ? 2.0 : 0.0);

  output.position =
      float4(output.uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0, 1);

  return output;
}
