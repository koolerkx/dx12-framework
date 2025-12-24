#include "basic_type.hlsli"

struct PushConstants {
  uint textureIndex;
};
ConstantBuffer<PushConstants> cb : register(b0);

float4 main(BasicType input) : SV_TARGET {
  return textures[cb.textureIndex].Sample(sampler0, input.uv);
}
