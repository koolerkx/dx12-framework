#include "root_signature_builder.h"

#include <iostream>
#include <stdexcept>


ComPtr<ID3D12RootSignature> RootSignatureBuilder::Build(ID3D12Device* device) {
  desc_.NumParameters = static_cast<UINT>(parameters_.size());
  desc_.pParameters = parameters_.empty() ? nullptr : parameters_.data();
  desc_.NumStaticSamplers = static_cast<UINT>(samplers_.size());
  desc_.pStaticSamplers = samplers_.empty() ? nullptr : samplers_.data();

  ComPtr<ID3DBlob> signature;
  ComPtr<ID3DBlob> error;
  HRESULT hr = D3D12SerializeRootSignature(&desc_, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error);

  if (FAILED(hr)) {
    if (error) {
      std::cerr << (char*)error->GetBufferPointer() << std::endl;
    }
    throw std::runtime_error("Failed to serialize root signature");
  }

  ComPtr<ID3D12RootSignature> rootSig;
  hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSig));

  if (FAILED(hr)) {
    throw std::runtime_error("Failed to create root signature");
  }

  return rootSig;
}

RootSignatureBuilder& RootSignatureBuilder::AddDescriptorTable(
  D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors, UINT baseShaderRegister, D3D12_SHADER_VISIBILITY visibility) {
  // Descriptor Range
  D3D12_DESCRIPTOR_RANGE range = {};
  range.RangeType = rangeType;
  range.NumDescriptors = numDescriptors;
  range.BaseShaderRegister = baseShaderRegister;
  range.RegisterSpace = 0;
  range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
  ranges_.push_back(range);

  // Root Parameter
  D3D12_ROOT_PARAMETER param = {};
  param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  param.DescriptorTable.NumDescriptorRanges = 1;
  param.DescriptorTable.pDescriptorRanges = &ranges_.back();
  param.ShaderVisibility = visibility;
  parameters_.push_back(param);

  return *this;
}
