// ============================================================================
// Sprite Instanced World Pixel Shader (Transparent Version)
// Purpose: Hardware instanced rendering for world-space transparent text/sprites
// Samples texture and applies per-instance color tint with full alpha blending
// NOTE: No alpha clipping/discard - uses proper alpha blending instead
// ============================================================================

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

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

float4 main(PSIN input) : SV_TARGET {
  // Sample albedo texture using bindless index
  float4 texColor = g_Textures[g_MaterialData.albedoTextureIndex].Sample(g_Sampler, input.uv);

  // Apply per-instance color tint
  float4 finalColor = texColor * input.color;

  // No alpha clipping - let alpha blending handle transparency
  // This allows smooth semi-transparent rendering
  return finalColor;
}
