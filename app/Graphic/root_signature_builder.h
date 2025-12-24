#include <d3d12.h>

#include <deque>
#include <vector>

#include "sampler_builder.h"
#include "types.h"

class RootSignatureBuilder {
 public:
  RootSignatureBuilder() {
    desc_.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
  }

  RootSignatureBuilder& AllowInputLayout() {
    desc_.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    return *this;
  }

  RootSignatureBuilder& AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
    UINT numDescriptors,
    UINT baseShaderRegister,
    UINT registerSpace = 0,
    D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

  RootSignatureBuilder& AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& samplerDesc) {
    samplers_.push_back(samplerDesc);
    return *this;
  }

  RootSignatureBuilder& AddStaticSampler(const SamplerBuilder& builder) {
    samplers_.push_back(builder.Get());
    return *this;
  }

  RootSignatureBuilder& Add32BitConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace = 0) {
    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.Num32BitValues = num32BitValues;
    param.Constants.ShaderRegister = shaderRegister;
    param.Constants.RegisterSpace = registerSpace;
    param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    parameters_.push_back(param);
    return *this;
  }

  RootSignatureBuilder& AddRootCBV(UINT shaderRegister, UINT registerSpace = 0) {
    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    param.Descriptor.ShaderRegister = shaderRegister;
    param.Descriptor.RegisterSpace = registerSpace;
    param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    parameters_.push_back(param);
    return *this;
  }

  ComPtr<ID3D12RootSignature> Build(ID3D12Device* device);

 private:
  D3D12_ROOT_SIGNATURE_DESC desc_;
  std::vector<D3D12_ROOT_PARAMETER> parameters_;
  std::deque<D3D12_DESCRIPTOR_RANGE> ranges_;  // use deque prevent pointer dangling
  std::vector<D3D12_STATIC_SAMPLER_DESC> samplers_;
};
