struct NormalViewCB {
  uint srcSrvIndex;
};
ConstantBuffer<NormalViewCB> g_NormalViewCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  float3 normal = g_Textures[g_NormalViewCB.srcSrvIndex].Sample(g_Samplers[0], input.uv).rgb;
  return float4(normal * 0.5 + 0.5, 1.0);
}
