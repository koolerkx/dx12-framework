struct LinearDepthViewCB {
  uint srcSrvIndex;
  float nearPlane;
  float farPlane;
  uint _padding;
};
ConstantBuffer<LinearDepthViewCB> g_LinearDepthViewCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  float depth = g_Textures[g_LinearDepthViewCB.srcSrvIndex].Sample(g_Samplers[0], input.uv).a;
  float normalized = saturate((depth - g_LinearDepthViewCB.nearPlane) / (g_LinearDepthViewCB.farPlane - g_LinearDepthViewCB.nearPlane));
  return float4(normalized, normalized, normalized, 1.0);
}
