struct VSIN {
  float4 pos : POSITION;
  float2 uv : TEXCOORD;
};

struct BasicType {
  float4 svpos : SV_POSITION;
  float2 uv : TEXCOORD;
};

Texture2D textures[] : register(t0, space1);
SamplerState sampler0 : register(s0);