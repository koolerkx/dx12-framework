// Bindless SMAA bridge: redirects SMAA.hlsl sampling macros to our bindless sampler array
#ifndef SMAA_COMMON_HLSLI
#define SMAA_COMMON_HLSLI

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

#define SMAA_CUSTOM_SL
#define SMAA_PRESET_HIGH

#define SMAATexture2D(tex) Texture2D tex
#define SMAATexturePass2D(tex) tex
#define SMAASampleLevelZero(tex, coord) tex.SampleLevel(g_Samplers[SAMPLER_LINEAR_CLAMP], coord, 0)
#define SMAASampleLevelZeroPoint(tex, coord) tex.SampleLevel(g_Samplers[SAMPLER_POINT_CLAMP], coord, 0)
#define SMAASampleLevelZeroOffset(tex, coord, offset) tex.SampleLevel(g_Samplers[SAMPLER_LINEAR_CLAMP], coord, 0, offset)
#define SMAASample(tex, coord) tex.Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], coord)
#define SMAASamplePoint(tex, coord) tex.Sample(g_Samplers[SAMPLER_POINT_CLAMP], coord)
#define SMAASampleOffset(tex, coord, offset) tex.Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], coord, offset)
#define SMAA_FLATTEN [flatten]
#define SMAA_BRANCH [branch]
#define SMAAGather(tex, coord) tex.Gather(g_Samplers[SAMPLER_LINEAR_CLAMP], coord, 0)

#include "SMAA.hlsl"

#endif
