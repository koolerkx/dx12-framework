// Override before include
// #define FRAME_CB_SLOT b0
// #include "ConstantBuffer/frame_cb.hlsli"

#ifndef FRAME_CB_HLSLI
#define FRAME_CB_HLSLI

#ifndef FRAME_CB_SLOT
#define FRAME_CB_SLOT b0
#endif

struct FrameCB {
  row_major float4x4 view;
  row_major float4x4 proj;
  row_major float4x4 viewProj;
  row_major float4x4 invView;
  row_major float4x4 invProj;
  float3 cameraPos;
  float time;
  float2 screenSize;
  float2 _padding0;
};
ConstantBuffer<FrameCB> g_FrameCB : register(FRAME_CB_SLOT);

#endif
