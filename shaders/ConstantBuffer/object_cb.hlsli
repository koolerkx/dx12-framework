// Override before include
// #define OBJECT_CB_SLOT b1
// #include "ConstantBuffer/object_cb.hlsli"

#ifndef OBJECT_CB_HLSLI
#define OBJECT_CB_HLSLI

#define OBJECT_FLAG_LIT (1u << 0)
#define OBJECT_FLAG_OPAQUE (1u << 1)
#define OBJECT_FLAG_RECEIVE_SHADOW (1u << 2)

#ifndef OBJECT_CB_SLOT
#define OBJECT_CB_SLOT b1
#endif

struct ObjectCB {
  row_major float4x4 world;
  row_major float4x4 worldViewProj;
  row_major float4x4 normalMatrix;
  float4 color;
  float2 uvOffset;
  float2 uvScale;
  uint materialDescriptorIndex;
  uint flags;
  uint _pad[2];
};
ConstantBuffer<ObjectCB> g_ObjectCB : register(OBJECT_CB_SLOT);

#endif
