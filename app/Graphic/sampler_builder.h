#pragma once

#include <d3d12.h>

class SamplerBuilder {
 public:
  explicit SamplerBuilder(UINT shaderRegister) {
    desc_.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    desc_.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc_.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc_.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    desc_.MipLODBias = 0.0f;
    desc_.MaxAnisotropy = 16;
    desc_.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    desc_.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    desc_.MinLOD = 0.0f;
    desc_.MaxLOD = D3D12_FLOAT32_MAX;
    desc_.ShaderRegister = shaderRegister;
    desc_.RegisterSpace = 0;
    desc_.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
  }

  SamplerBuilder& SetFilter(D3D12_FILTER filter) {
    desc_.Filter = filter;
    return *this;
  }

  SamplerBuilder& SetAddressMode(D3D12_TEXTURE_ADDRESS_MODE mode) {
    desc_.AddressU = mode;
    desc_.AddressV = mode;
    desc_.AddressW = mode;
    return *this;
  }

  SamplerBuilder& SetAddressModes(D3D12_TEXTURE_ADDRESS_MODE u, D3D12_TEXTURE_ADDRESS_MODE v, D3D12_TEXTURE_ADDRESS_MODE w) {
    desc_.AddressU = u;
    desc_.AddressV = v;
    desc_.AddressW = w;
    return *this;
  }

  SamplerBuilder& SetBorderColor(D3D12_STATIC_BORDER_COLOR color) {
    desc_.BorderColor = color;
    return *this;
  }

  SamplerBuilder& SetLODRange(float minLOD, float maxLOD) {
    desc_.MinLOD = minLOD;
    desc_.MaxLOD = maxLOD;
    return *this;
  }

  SamplerBuilder& SetComparisonFunc(D3D12_COMPARISON_FUNC func) {
    desc_.ComparisonFunc = func;
    return *this;
  }

  SamplerBuilder& SetShaderVisibility(D3D12_SHADER_VISIBILITY visibility) {
    desc_.ShaderVisibility = visibility;
    return *this;
  }

  SamplerBuilder& SetMaxAnisotropy(UINT maxAnisotropy) {
    desc_.MaxAnisotropy = maxAnisotropy;
    return *this;
  }

  const D3D12_STATIC_SAMPLER_DESC& Get() const {
    return desc_;
  }

  operator D3D12_STATIC_SAMPLER_DESC() const {
    return desc_;
  }

 private:
  D3D12_STATIC_SAMPLER_DESC desc_;
};

namespace SamplerPresets {

SamplerBuilder CreatePointSampler(UINT shaderRegister,
  D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
  D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);

SamplerBuilder CreateLinearSampler(UINT shaderRegister,
  D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
  D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);

SamplerBuilder CreateAnisotropicSampler(UINT shaderRegister,
  UINT maxAnisotropy = 16,
  D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
  D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);

SamplerBuilder CreateComparisonSampler(UINT shaderRegister,
  D3D12_COMPARISON_FUNC comparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
  D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);
}  // namespace SamplerPresets
