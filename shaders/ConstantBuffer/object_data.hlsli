#ifndef OBJECT_DATA_HLSLI
#define OBJECT_DATA_HLSLI

#define OBJECT_FLAG_LIT (1u << 0)
#define OBJECT_FLAG_OPAQUE (1u << 1)
#define OBJECT_FLAG_RECEIVE_SHADOW (1u << 2)

struct ObjectData {
  row_major float4x4 world;
  float4 color;
  float2 uvOffset;
  float2 uvScale;
  float4 overlayColor;
  uint materialDescriptorIndex;
  uint flags;
  uint _pad[2];
};

StructuredBuffer<ObjectData> g_ObjectBuffer : register(t0, space3);

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
