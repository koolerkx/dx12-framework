// ============================================================================
// Sprite Instanced World Vertex Shader (Transparent Version)
// Purpose: Hardware instanced rendering for world-space transparent text/sprites
// Uses perspective projection with 3D world coordinates (Y-up)
// ============================================================================

// === Constant Buffers ===
cbuffer FrameCB : register(b0) {
  float4x4 g_View;
  float4x4 g_Proj;
  float4x4 g_ViewProj;
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

  // Slot 1: Instance Data (Per-Instance, Glyph/Sprite Data)
  // Note: float4x4 requires 4 semantic indices
  // CPU stores: XMMatrixTranspose(world) - aligned with FrameCB matrix format
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

  // Reconstruct transposed world matrix from 4 float4 rows
  // CPU stores: XMMatrixTranspose(world) to align with CBV convention
  // But unlike CBV (which has column-major interpretation that cancels transpose),
  // Vertex Buffer passes raw bytes - so we need to transpose back here.
  float4x4 instanceWorld_T = float4x4(
    input.instanceWorld0,
    input.instanceWorld1,
    input.instanceWorld2,
    input.instanceWorld3
  );
  float4x4 instanceWorld = transpose(instanceWorld_T);

  // === Matrix Multiplication (Row-Vector Format) ===
  // Same as legacy basic.vs.hlsl: mul(pos, matrix)
  // Now both instanceWorld and g_ViewProj are in original (non-transposed) form
  float4 localPos = float4(input.position, 1.0f);
  float4 worldPos = mul(localPos, instanceWorld);
  output.position = mul(worldPos, g_ViewProj);

  // Apply per-instance UV transform: scale first, then offset
  output.uv = input.uv * input.instanceUVScale + input.instanceUVOffset;

  // Pass through per-instance color
  output.color = input.instanceColor;

  return output;
}
