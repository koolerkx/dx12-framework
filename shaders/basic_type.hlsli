struct VSIN {
  float3 pos : POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

struct BasicType {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

// === Constant Buffers ===
cbuffer FrameCB : register(b0) {
  row_major float4x4 g_View;
  row_major float4x4 g_Proj;
  row_major float4x4 g_ViewProj;
  float3 g_CameraPos;
  float g_Time;
  float2 g_ScreenSize;
  float2 _padding0;
};

cbuffer ObjectCB : register(b1) {
  row_major float4x4 g_World;
  row_major float4x4 g_WorldViewProj;
  float4 g_ObjectColor;
  float2 g_UVOffset;   // UV offset for atlas/sprite sheet
  float2 g_UVScale;    // UV scale (1,1 = full texture)
  uint g_SamplerIndex; // Sampler index for bindless sampler array
  uint3 _padding1;     // Padding to maintain alignment
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