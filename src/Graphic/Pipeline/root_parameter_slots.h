#pragma once
#include <cstdint>

// Root parameter slot indices for type-safe binding
// These are the fixed slots in your root signature that all shaders share
namespace RootSlot {

// Root constants and CBVs
enum class ConstantBuffer : uint32_t {
  Frame = 0,     // FrameCB - per-frame constants (view, proj, camera, time, etc)
  Object = 1,    // ObjectCB - per-object constants (world matrix, color, etc)
  Light = 2,     // LightingCB - lighting data
  Material = 3,  // Reserved (previously MaterialCB)
  Shadow = 4,    // ShadowCB - shadow mapping data (b4)
  Custom = 9,    // CustomCB - per-object custom data (b5)
};

// Descriptor tables
enum class DescriptorTable : uint32_t {
  GlobalSRVs = 5,  // Global bindless texture array (t0, space1)
  Samplers = 6,    // Bindless sampler array (s0, space0)
};

// Root SRVs
enum class ShaderResource : uint32_t {
  PointLights = 7,           // StructuredBuffer<PointLight> (t0, space2)
  ObjectBuffer = 8,          // StructuredBuffer<ObjectData> (t0, space3)
  MeshDescriptors = 10,      // StructuredBuffer<MeshDescriptor> (t0, space4)
  MaterialDescriptors = 11,  // StructuredBuffer<MaterialDescriptor> (t0, space5)
};

// Bindless sampler heap indices (see shaders/ConstantBuffer/sampler.hlsli)
enum class SamplerIndex : uint32_t {
  PointWrap = 0,        // SAMPLER_POINT_WRAP
  LinearWrap = 1,       // SAMPLER_LINEAR_WRAP
  AnisotropicWrap = 2,  // SAMPLER_ANISOTROPIC_WRAP
  PointClamp = 3,       // SAMPLER_POINT_CLAMP
  LinearClamp = 4,      // SAMPLER_LINEAR_CLAMP
};

// Helper functions to convert enum to uint32_t for API calls
inline constexpr uint32_t ToIndex(ConstantBuffer slot) {
  return static_cast<uint32_t>(slot);
}

inline constexpr uint32_t ToIndex(DescriptorTable slot) {
  return static_cast<uint32_t>(slot);
}

inline constexpr uint32_t ToIndex(ShaderResource slot) {
  return static_cast<uint32_t>(slot);
}

inline constexpr uint32_t ToIndex(SamplerIndex slot) {
  return static_cast<uint32_t>(slot);
}

}  // namespace RootSlot
