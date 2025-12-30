// === Constant Buffers ===
cbuffer FrameCB : register(b0) {
  float4x4 g_View;
  float4x4 g_Proj;
  float4x4 g_ViewProj;
  float3 g_CameraPos;
  float g_Time;
  float2 g_ScreenSize;
  float2 _padding0;
};

cbuffer ObjectCB : register(b1) {
  float4x4 g_World;
  float4x4 g_WorldViewProj;
  float4 g_ObjectColor;
};

// Material data from root constants
struct MaterialData {
  uint albedoTextureIndex;
  uint normalTextureIndex;
  uint metallicRoughnessIndex;
  uint flags;
};
ConstantBuffer<MaterialData> g_MaterialData : register(b3);

// === Bindless Resources ===
Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Sampler : register(s0);

// === Vertex Shader ===
struct VSIN {
  float3 position : POSITION;
  float2 uv : TEXCOORD;
};

struct VSOUT {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD0;
};

VSOUT main(VSIN input) {
  VSOUT output;

  output.position = mul(float4(input.position, 1.0f), g_WorldViewProj);
  output.uv = input.uv;

  return output;
}