struct BloomDownCB {
  uint sourceSrvIndex;
  float2 texelSize;
  float threshold;
};
ConstantBuffer<BloomDownCB> g_CB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  Texture2D src = g_Textures[g_CB.sourceSrvIndex];
  SamplerState samp = g_Samplers[4];
  float2 uv = input.uv;
  float2 offset = g_CB.texelSize * 0.5;

  float4 sum = src.Sample(samp, uv) * 4.0;
  sum += src.Sample(samp, uv + float2(-offset.x, -offset.y));
  sum += src.Sample(samp, uv + float2(+offset.x, +offset.y));
  sum += src.Sample(samp, uv + float2(+offset.x, -offset.y));
  sum += src.Sample(samp, uv + float2(-offset.x, +offset.y));
  sum /= 8.0;

  if (g_CB.threshold > 0.0) {
    float brightness = max(sum.r, max(sum.g, sum.b));
    float contribution = max(0, brightness - g_CB.threshold) / max(brightness, 0.00001);
    sum.rgb *= contribution;
  }

  return sum;
}
