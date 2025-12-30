#pragma once
#include <d3d12.h>

#include <cstdint>
#include <string>

#include "Core/types.h"

// Material defines a complete rendering state (PSO + Root Signature + additional config)
// Each material represents a unique combination of shaders and render state
class Material {
 public:
  Material() = default;
  Material(const std::string& name, ID3D12RootSignature* root_signature, ID3D12PipelineState* pipeline_state, uint32_t sort_key = 0)
      : name_(name), root_signature_(root_signature), pipeline_state_(pipeline_state), sort_key_(sort_key) {
  }

  const std::string& GetName() const {
    return name_;
  }

  ID3D12RootSignature* GetRootSignature() const {
    return root_signature_.Get();
  }

  ID3D12PipelineState* GetPipelineState() const {
    return pipeline_state_.Get();
  }

  // Sort key for optimizing PSO switches
  // Lower values render first
  // Group materials with same PSO together for better performance
  uint32_t GetSortKey() const {
    return sort_key_;
  }

  void SetSortKey(uint32_t key) {
    sort_key_ = key;
  }

  bool IsValid() const {
    return root_signature_ != nullptr && pipeline_state_ != nullptr;
  }

 private:
  std::string name_;
  ComPtr<ID3D12RootSignature> root_signature_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  uint32_t sort_key_ = 0;  // For sorting to minimize PSO switches
};

// Material instance with per-instance data
struct MaterialInstance {
  const Material* material = nullptr;

  // Material parameters (texture indices, tint colors, etc)
  uint32_t albedo_texture_index = 0;
  uint32_t normal_texture_index = 0;
  uint32_t metallic_roughness_index = 0;

  // Material flags
  bool use_alpha_test = false;
  bool double_sided = false;

  // Helpers
  bool IsValid() const {
    return material != nullptr && material->IsValid();
  }

  uint32_t GetSortKey() const {
    return material ? material->GetSortKey() : UINT32_MAX;
  }
};
