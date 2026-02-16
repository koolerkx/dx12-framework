#ifndef INSTANCE_CB_HLSLI
#define INSTANCE_CB_HLSLI

struct InstanceData {
  row_major float4x4 world;
  row_major float4x4 normalMatrix;
  float4 color;
  float4 overlayColor;
};

StructuredBuffer<InstanceData> g_InstanceBuffer : register(t0, space3);

#endif
