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

  // Constructor with 64-bit sort key for RS + PSO sorting
  Material(const std::string& name,
    ID3D12RootSignature* root_signature,
    ID3D12PipelineState* pipeline_state,
    uint64_t sort_key = 0)  // Default parameter for backward compatibility
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

  // Sort key for optimizing RS + PSO switches
  // High 32 bits: Root Signature hash (groups by RS)
  // Low 32 bits: Pipeline State hash (groups by PSO within same RS)
  uint64_t GetSortKey() const {
    return sort_key_;
  }

  // Extract RS hash from sort key
  uint32_t GetRootSignatureKey() const {
    return static_cast<uint32_t>(sort_key_ >> 32);
  }

  // Extract PSO hash from sort key
  uint32_t GetPSOKey() const {
    return static_cast<uint32_t>(sort_key_ & 0xFFFFFFFF);
  }

  void SetSortKey(uint64_t key) {
    sort_key_ = key;
  }

  bool IsValid() const {
    return root_signature_ != nullptr && pipeline_state_ != nullptr;
  }

  void SetInstancingSupport(bool supports) {
    supports_instancing_ = supports;
  }

  [[nodiscard]] bool SupportsInstancing() const {
    return supports_instancing_;
  }

  void SetStructuredInstancingSupport(bool supports) {
    supports_structured_instancing_ = supports;
  }

  [[nodiscard]] bool SupportsStructuredInstancing() const {
    return supports_structured_instancing_;
  }

 private:
  std::string name_;
  ComPtr<ID3D12RootSignature> root_signature_;
  ComPtr<ID3D12PipelineState> pipeline_state_;
  uint64_t sort_key_ = 0;  // 64-bit: [32-bit RS hash | 32-bit PSO hash]
  bool supports_instancing_ = false;
  bool supports_structured_instancing_ = false;
};

