#pragma once
#include <cstdint>

namespace Graphics {

// Root Signature Preset
enum class RSPreset : uint8_t {
  Standard,  // FrameCB + ObjectCB + MaterialData + SRVs + Samplers
  Deferred,  // GBuffer render targets
  Compute,   // Compute shader

  // Future extensions
  // Terrain,  // 地形渲染（額外的 height map CBV）
  // Water,    // 水體渲染（額外的 wave params CBV）

  Count
};

// Shader Family
enum class ShaderFamily : uint8_t {
  Sprite,       // RSPreset::Standard
  Mesh,         // RSPreset::Standard
  Text,         // RSPreset::Standard
  Deferred,     // RSPreset::Deferred
  PostProcess,  // RSPreset::Standard
  Compute,      // RSPreset::Compute

  Count
};

// Shader ID
enum class ShaderID : uint32_t {
  // === Implemented Shaders ===
  Sprite = 0,
  SpriteInstancedUI = 1,
  SpriteInstancedWorld = 2,
  SpriteInstancedWorldTransparent = 3,
  Basic3D = 4,
  DebugLine = 5,

  // === Future Shaders - Reserved for future implementation ===
  MeshStatic = 10,
  MeshSkinned = 11,
  MeshPBR = 12,

  GBufferOpaque = 20,
  GBufferAlphaTest = 21,
  GBufferSkinned = 22,
  DeferredDirectionalLight = 30,
  DeferredPointLight = 31,
  DeferredSpotLight = 32,

  PostProcessToneMap = 40,
  PostProcessBloom = 41,
  PostProcessSSAO = 42,

  ComputeParticleUpdate = 50,
  ComputeCulling = 51,

  Count
};

}  // namespace Graphics
