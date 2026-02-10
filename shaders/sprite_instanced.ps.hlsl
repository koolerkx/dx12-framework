cbuffer ObjectCB : register(b1) {
  row_major float4x4 g_World;
  row_major float4x4 g_WorldViewProj;
  float4 g_ObjectColor;
  float2 g_UVOffset;
  float2 g_UVScale;
  uint g_SamplerIndex;
  uint3 _padding1;
};

struct MaterialData {
  uint albedoTextureIndex;
  uint normalTextureIndex;
  uint metallicRoughnessIndex;
  uint flags;
};
ConstantBuffer<MaterialData> g_MaterialData : register(b3);

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

  if ((g_MaterialData.flags & 1u) && finalColor.a < 0.01f) {
    discard;
  }

  return finalColor;
}
