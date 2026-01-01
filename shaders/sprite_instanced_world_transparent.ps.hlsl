// ============================================================================
// Sprite Instanced World Pixel Shader (Transparent Version)
// Purpose: Hardware instanced rendering for world-space transparent
// text/sprites Samples texture and applies per-instance color tint with full
// alpha blending NOTE: No alpha clipping/discard - uses proper alpha blending
// instead
// ============================================================================

cbuffer ObjectCB : register(b1) {
  float4x4 g_World;
  float4x4 g_WorldViewProj;
  float4 g_ObjectColor;
  float2 g_UVOffset;
  float2 g_UVScale;
  uint g_SamplerIndex;
  uint3 _padding1;
};

// Material data from root constants
struct MaterialData {
  uint albedoTextureIndex;
  uint normalTextureIndex;
  uint metallicRoughnessIndex;
  uint flags;
};
ConstantBuffer<MaterialData> g_MaterialData : register(b3);

// Bindless Resources
Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

float4 main(PSIN input) : SV_TARGET {
  float4 texColor = g_Textures[g_MaterialData.albedoTextureIndex].Sample(
      g_Samplers[g_SamplerIndex], input.uv);

  float4 finalColor = texColor * input.color;

  return finalColor;
}
