#include "ConstantBuffer/frame_cb.hlsli"

struct SSAOCB {
  uint normalDepthSrvIndex;
  float radius;
  float bias;
  float intensity;
  uint sampleCount;
  uint3 _pad;
};
ConstantBuffer<SSAOCB> g_SSAOCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

static const float3 KERNEL_SAMPLES[32] = {
    float3(0.0299, 0.0355, 0.0114),   float3(-0.0262, -0.0197, 0.0611),
    float3(0.0747, -0.0171, 0.0283),  float3(-0.0484, 0.0617, 0.0381),
    float3(0.0185, -0.0834, 0.0438),  float3(0.0937, 0.0293, 0.0510),
    float3(-0.0402, 0.0197, 0.0897),  float3(0.0810, -0.0791, 0.0221),
    float3(-0.0963, -0.0393, 0.0633), float3(0.0337, 0.1162, 0.0120),
    float3(0.1237, -0.0523, 0.0397),  float3(-0.0893, 0.0928, 0.0683),
    float3(-0.0142, -0.1372, 0.0544), float3(0.1458, 0.0117, 0.0876),
    float3(-0.1089, -0.1076, 0.0467), float3(0.0692, 0.1549, 0.0324),
    float3(0.1831, -0.0397, 0.0753),  float3(-0.1292, 0.1355, 0.0891),
    float3(-0.0578, -0.1876, 0.1032), float3(0.2104, 0.0656, 0.0418),
    float3(-0.1694, -0.1573, 0.0832), float3(0.1134, 0.2108, 0.0567),
    float3(0.2437, -0.1028, 0.1186),  float3(-0.1892, 0.1902, 0.1341),
    float3(-0.0913, -0.2691, 0.0766), float3(0.2847, 0.0345, 0.1532),
    float3(-0.2338, -0.2024, 0.1198), float3(0.1523, 0.2874, 0.0943),
    float3(0.3216, -0.1276, 0.0628),  float3(-0.2587, 0.2497, 0.1724),
    float3(-0.1246, -0.3418, 0.1088), float3(0.3572, 0.1187, 0.2043),
};

float Hash(float2 p) {
  float3 p3 = frac(float3(p.xyx) * 0.1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return frac((p3.x + p3.y) * p3.z);
}

float3 ReconstructViewPosition(float2 uv, float viewZ) {
  float2 ndc = uv * 2.0 - 1.0;
  ndc.y = -ndc.y;
  float4 viewRay = mul(float4(ndc, 1.0, 1.0), g_FrameCB.invProj);
  viewRay.xyz /= viewRay.w;
  return viewRay.xyz * (viewZ / viewRay.z);
}

float4 main(PSIN input) : SV_TARGET {
  float4 normalDepth =
      g_Textures[g_SSAOCB.normalDepthSrvIndex].Sample(g_Samplers[4], input.uv);
  float3 viewNormal = normalize(normalDepth.xyz);
  float viewZ = normalDepth.w;

  if (viewZ <= 0.0 || viewZ > 1000.0) {
    return 1.0;
  }

  float3 viewPos = ReconstructViewPosition(input.uv, viewZ);

  float randomAngle = Hash(input.position.xy) * 6.28318530718;
  float cosA = cos(randomAngle);
  float sinA = sin(randomAngle);

  float3 up = abs(viewNormal.y) < 0.999 ? float3(0, 1, 0) : float3(1, 0, 0);
  float3 tangent = normalize(cross(up, viewNormal));
  float3 bitangent = cross(viewNormal, tangent);

  float3 rotatedTangent = tangent * cosA + bitangent * sinA;
  float3 rotatedBitangent = -tangent * sinA + bitangent * cosA;
  float3x3 TBN = float3x3(rotatedTangent, rotatedBitangent, viewNormal);

  float occlusion = 0.0;
  uint count = min(g_SSAOCB.sampleCount, 32u);
  uint validSamples = 0;

  [loop] for (uint i = 0; i < count; ++i) {
    float3 sampleDir = mul(KERNEL_SAMPLES[i], TBN);
    float3 samplePos = viewPos + sampleDir * g_SSAOCB.radius;

    float4 clipPos = mul(float4(samplePos, 1.0), g_FrameCB.proj);
    clipPos.xy /= clipPos.w;
    float2 sampleUV = clipPos.xy * float2(0.5, -0.5) + 0.5;

    if (any(sampleUV < 0.0) || any(sampleUV > 1.0))
      continue;

    validSamples++;

    float sampledZ = g_Textures[g_SSAOCB.normalDepthSrvIndex]
                         .SampleLevel(g_Samplers[4], sampleUV, 0)
                         .w;

    float rangeCheck = smoothstep(
        g_SSAOCB.radius, g_SSAOCB.radius * 0.5, abs(samplePos.z - sampledZ));
    float delta = samplePos.z - g_SSAOCB.bias - sampledZ;
    occlusion += smoothstep(0.0, g_SSAOCB.bias * 2.0, delta) * rangeCheck;
  }

  float ao = 1.0 - (occlusion / float(max(validSamples, 1u))) * g_SSAOCB.intensity;
  return saturate(ao);
}
