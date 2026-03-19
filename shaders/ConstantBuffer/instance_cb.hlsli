#ifndef INSTANCE_CB_HLSLI
#define INSTANCE_CB_HLSLI

struct InstanceData {
  row_major float4x4 world;
  float4 color;
  float2 uvOffset;
  float2 uvScale;
  float4 overlayColor;
};

StructuredBuffer<InstanceData> g_InstanceBuffer : register(t0, space3);

// cofactor(M) / det(M) — equivalent to CPU world.Inverted().Transposed()
float3x3 ComputeNormalMatrix(float4x4 w) {
  float3x3 m = (float3x3)w;
  float3 c0 = cross(m[1], m[2]);
  float3 c1 = cross(m[2], m[0]);
  float3 c2 = cross(m[0], m[1]);
  float det = dot(m[0], c0);
  if (abs(det) < 1e-10f) return (float3x3)w;
  float s = rcp(det);
  return float3x3(c0 * s, c1 * s, c2 * s);
}

#endif
