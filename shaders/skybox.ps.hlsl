cbuffer ObjectCB : register(b1) {
  row_major float4x4 g_World;
  row_major float4x4 g_WorldViewProj;
  float4 g_ObjectColor;
  float2 g_UVOffset;
  float2 g_UVScale;
  uint g_SamplerIndex;
  uint3 _obj_padding;
};

cbuffer SkyboxCB : register(b2) {
  uint g_CubemapSrvIndex;
  uint3 _skybox_padding;
};

TextureCube g_CubeTextures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct SkyboxPSInput {
  float4 position : SV_POSITION;
  float3 local_pos : TEXCOORD0;
};

float4 main(SkyboxPSInput input) : SV_TARGET {
  if (g_CubemapSrvIndex == 0xFFFFFFFF) return g_ObjectColor;

  float3 color = g_CubeTextures[g_CubemapSrvIndex]
                     .Sample(g_Samplers[0], normalize(input.local_pos))
                     .rgb;
  return float4(color, 1.0);
}
