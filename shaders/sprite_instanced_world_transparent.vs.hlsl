// ============================================================================
// Sprite Instanced World Vertex Shader (Transparent Version)
// Purpose: Hardware instanced rendering for world-space transparent
// text/sprites Uses perspective projection with 3D world coordinates (Y-up)
// ============================================================================

// === Constant Buffers ===
cbuffer FrameCB : register(b0) {
  row_major float4x4 g_View;
  row_major float4x4 g_Proj;
  row_major float4x4 g_ViewProj;
  float3 g_CameraPos;
  float g_Time;
  float2 g_ScreenSize;
  float2 _padding0;
};

// Material data from root constants
struct MaterialData {
  uint albedoTextureIndex;
  uint normalTextureIndex;
  uint metallicRoughnessIndex;
  uint flags;
};
ConstantBuffer<MaterialData> g_MaterialData : register(b3);

// === Bindless Resources ===
Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Sampler : register(s0);

// === Vertex Shader Input ===
struct VSIN {
  // Slot 0: Mesh Data (Per-Vertex, Quad Vertices)
  float3 position : POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;

  // Slot 1: Instance Data (Per-Instance, Glyph/Sprite Data)
  float4 instanceWorld0 : INSTANCE_WORLD0;
  float4 instanceWorld1 : INSTANCE_WORLD1;
  float4 instanceWorld2 : INSTANCE_WORLD2;
  float4 instanceWorld3 : INSTANCE_WORLD3;
  float4 instanceColor : INSTANCE_COLOR;
  float2 instanceUVOffset : INSTANCE_UV_OFFSET;
  float2 instanceUVScale : INSTANCE_UV_SCALE;
};

struct VSOUT {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

VSOUT main(VSIN input) {
  VSOUT output;

  float4x4 instanceWorld = float4x4(input.instanceWorld0, input.instanceWorld1,
                                    input.instanceWorld2, input.instanceWorld3);

  // === Matrix Multiplication (Row-Vector Format) ===
  float4 localPos = float4(input.position, 1.0f);
  float4 worldPos = mul(localPos, instanceWorld);
  output.position = mul(worldPos, g_ViewProj);

  output.uv = input.uv * input.instanceUVScale + input.instanceUVOffset;
  output.color = input.color * input.instanceColor;

  return output;
}
