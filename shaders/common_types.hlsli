#ifndef COMMON_TYPES_HLSLI
#define COMMON_TYPES_HLSLI

struct VSIN {
  float3 position : POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

struct VS_IN_3D {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

#endif
