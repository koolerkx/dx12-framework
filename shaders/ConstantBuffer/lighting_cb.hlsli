#ifndef LIGHTING_CB_HLSLI
#define LIGHTING_CB_HLSLI

#ifndef LIGHTING_CB_SLOT
#define LIGHTING_CB_SLOT b2
#endif

struct LightingCB {
  float3 lightDirection;
  float lightIntensity;
  float3 directionalColor;
  float ambientIntensity;
  float3 ambientColor;
  uint pointLightCount;
};
ConstantBuffer<LightingCB> g_LightingCB : register(LIGHTING_CB_SLOT);

struct PointLight {
  float3 position;
  float intensity;
  float3 color;
  float radius;
  float falloff;
  uint enabled;
  float2 _padding;
};

StructuredBuffer<PointLight> g_PointLights : register(t0, space2);

float ComputePointLightAttenuation(float distance, float radius,
                                   float falloff) {
  float normalizedDist = distance / radius;
  float attenuation = 1.0 / (1.0 + falloff * normalizedDist * normalizedDist);
  float window = saturate(1.0 - pow(normalizedDist, 4.0));
  window = window * window;
  return attenuation * window;
}

void CalcPointLightContribution(float3 N, float3 V, float3 worldPos,
                                uint pointLightCount, float specularIntensity,
                                float specularPower, out float3 totalDiffuse,
                                out float3 totalSpecular) {
  totalDiffuse = float3(0, 0, 0);
  totalSpecular = float3(0, 0, 0);

  [loop] for (uint i = 0; i < pointLightCount; i++) {
    if (g_PointLights[i].enabled == 0)
      continue;

    float3 toLight = g_PointLights[i].position - worldPos;
    float distance = length(toLight);

    if (distance < 1e-4 || distance > g_PointLights[i].radius)
      continue;

    float3 L = toLight / distance;
    float NdotL = saturate(dot(N, L));

    if (NdotL > 0.0) {
      float attenuation = ComputePointLightAttenuation(
          distance, g_PointLights[i].radius, g_PointLights[i].falloff);

      float3 lightColor =
          g_PointLights[i].color * g_PointLights[i].intensity * attenuation;

      totalDiffuse += lightColor * NdotL;

      float3 H = normalize(L + V);
      float spec = pow(saturate(dot(N, H)), specularPower);
      totalSpecular += lightColor * specularIntensity * spec;
    }
  }
}

#endif
