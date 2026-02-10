// Override before include
// #define OBJECT_CB_SLOT b1
// #include "ConstantBuffer/object_cb.hlsli"

#ifndef OBJECT_CB_HLSLI
#define OBJECT_CB_HLSLI

#ifndef OBJECT_CB_SLOT
#define OBJECT_CB_SLOT b1
#endif

struct ObjectCB {
  row_major float4x4 world;
  row_major float4x4 worldViewProj;
  float4 color;
  float2 uvOffset;
  float2 uvScale;
  uint samplerIndex;
  uint3 _padding1;
};
ConstantBuffer<ObjectCB> g_ObjectCB : register(OBJECT_CB_SLOT);

#endif
