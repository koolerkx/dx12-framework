#ifndef SAMPLER_HLSLI
#define SAMPLER_HLSLI

SamplerState g_Samplers[] : register(s0, space0);

#define SAMPLER_POINT_WRAP       0  // s0
#define SAMPLER_LINEAR_WRAP      1  // s1
#define SAMPLER_ANISOTROPIC_WRAP 2  // s2
#define SAMPLER_POINT_CLAMP      3  // s3
#define SAMPLER_LINEAR_CLAMP     4  // s4

#endif
