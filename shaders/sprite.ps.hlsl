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
  float2 g_UVOffset;  // UV offset for atlas/sprite sheet
  float2 g_UVScale;   // UV scale (1,1 = full texture)
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
struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD0;
};

float4 main(PSIN input) : SV_TARGET {
  float4 texColor = g_Textures[g_MaterialData.albedoTextureIndex].Sample(g_Sampler, input.uv);
  return texColor * g_ObjectColor;
}