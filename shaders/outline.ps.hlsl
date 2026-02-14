#include "ConstantBuffer/frame_cb.hlsli"

// Dot product default: immune to grazing-angle false edges.
// Sobel: thicker lines but artifacts on curved surfaces.
// #define USE_SOBEL_EDGE

struct OutlineCB {
  uint normalDepthSrvIndex;
  uint sceneSrvIndex;
  float depthWeight;
  float normalWeight;
  float edgeThreshold;
  float depthFalloff;
  float thickness;
  uint _pad0;
  float3 outlineColor;
  uint _pad1;
};
ConstantBuffer<OutlineCB> g_OutlineCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 SampleNormalDepth(float2 uv) {
  return g_Textures[g_OutlineCB.normalDepthSrvIndex].SampleLevel(g_Samplers[4],
                                                                 uv, 0);
}

float DepthDiscontinuity(float2 uv, float2 o, float centerDepth) {
  float tl = SampleNormalDepth(uv + float2(-o.x, o.y)).w;
  float tc = SampleNormalDepth(uv + float2(0.0, o.y)).w;
  float tr = SampleNormalDepth(uv + float2(o.x, o.y)).w;
  float ml = SampleNormalDepth(uv + float2(-o.x, 0.0)).w;
  float mr = SampleNormalDepth(uv + float2(o.x, 0.0)).w;
  float bl = SampleNormalDepth(uv + float2(-o.x, -o.y)).w;
  float bc = SampleNormalDepth(uv + float2(0.0, -o.y)).w;
  float br = SampleNormalDepth(uv + float2(o.x, -o.y)).w;

  float gx = -tl - 2.0 * ml - bl + tr + 2.0 * mr + br;
  float gy = -tl - 2.0 * tc - tr + bl + 2.0 * bc + br;

  return sqrt(gx * gx + gy * gy) / max(centerDepth, 0.001);
}

#ifdef USE_SOBEL_EDGE

float NormalEdgeSobel(float2 uv, float2 o, float centerDepth) {
  float3 tl = SampleNormalDepth(uv + float2(-o.x, o.y)).xyz;
  float3 tc = SampleNormalDepth(uv + float2(0.0, o.y)).xyz;
  float3 tr = SampleNormalDepth(uv + float2(o.x, o.y)).xyz;
  float3 ml = SampleNormalDepth(uv + float2(-o.x, 0.0)).xyz;
  float3 mr = SampleNormalDepth(uv + float2(o.x, 0.0)).xyz;
  float3 bl = SampleNormalDepth(uv + float2(-o.x, -o.y)).xyz;
  float3 bc = SampleNormalDepth(uv + float2(0.0, -o.y)).xyz;
  float3 br = SampleNormalDepth(uv + float2(o.x, -o.y)).xyz;

  float3 gx = -tl - 2.0 * ml - bl + tr + 2.0 * mr + br;
  float3 gy = -tl - 2.0 * tc - tr + bl + 2.0 * bc + br;

  return sqrt(dot(gx, gx) + dot(gy, gy)) / max(centerDepth, 0.001);
}

#else

float NormalEdgeDotProduct(float3 centerNormal, float2 uv, float2 o) {
  float3 nT = normalize(SampleNormalDepth(uv + float2(0, o.y)).xyz);
  float3 nB = normalize(SampleNormalDepth(uv + float2(0, -o.y)).xyz);
  float3 nL = normalize(SampleNormalDepth(uv + float2(-o.x, 0)).xyz);
  float3 nR = normalize(SampleNormalDepth(uv + float2(o.x, 0)).xyz);

  float minDot = min(min(dot(centerNormal, nT), dot(centerNormal, nB)),
                     min(dot(centerNormal, nL), dot(centerNormal, nR)));

  return 1.0 - saturate(minDot);
}

#endif

float4 main(PSIN input) : SV_TARGET {
  float2 texelSize = 1.0 / g_FrameCB.screenSize;

  float4 centerSample = SampleNormalDepth(input.uv);
  float depth = centerSample.w;
  float3 sceneColor = g_Textures[g_OutlineCB.sceneSrvIndex]
                          .SampleLevel(g_Samplers[4], input.uv, 0)
                          .rgb;

  if (depth <= 0.0 || dot(centerSample.xyz, centerSample.xyz) < 0.001) {
    return float4(sceneColor, 1.0);
  }

  float3 centerNormal = normalize(centerSample.xyz);
  float2 sampleOffset = texelSize * g_OutlineCB.thickness;

  float depthEdge = DepthDiscontinuity(input.uv, sampleOffset, depth);

#ifdef USE_SOBEL_EDGE
  float normalEdge = NormalEdgeSobel(input.uv, sampleOffset, depth);
#else
  float normalEdge = NormalEdgeDotProduct(centerNormal, input.uv, sampleOffset);
#endif

  float edge = depthEdge * g_OutlineCB.depthWeight +
               normalEdge * g_OutlineCB.normalWeight;

  edge = smoothstep(g_OutlineCB.edgeThreshold, g_OutlineCB.edgeThreshold * 2.0,
                    edge);
  edge *= 1.0 / (1.0 + depth * g_OutlineCB.depthFalloff);
  edge = saturate(edge);

  float3 result = lerp(sceneColor, g_OutlineCB.outlineColor, edge);

  return float4(result, 1.0);
}
