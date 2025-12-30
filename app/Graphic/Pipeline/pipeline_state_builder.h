#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>

#include <vector>

#include "Core/types.h"

enum class BlendMode { Opaque, AlphaBlend, Additive, Premultiplied, NonPremultiplied };

class PipelineStateBuilder {
 public:
  PipelineStateBuilder() {
    desc_ = {};

    // Sample Mask
    desc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    // Rasterizer State
    desc_.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    desc_.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    desc_.RasterizerState.FrontCounterClockwise = FALSE;
    desc_.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    desc_.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    desc_.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    desc_.RasterizerState.DepthClipEnable = TRUE;
    desc_.RasterizerState.MultisampleEnable = FALSE;
    desc_.RasterizerState.AntialiasedLineEnable = FALSE;
    desc_.RasterizerState.ForcedSampleCount = 0;
    desc_.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Blend State
    desc_.BlendState.AlphaToCoverageEnable = FALSE;
    desc_.BlendState.IndependentBlendEnable = FALSE;
    for (auto& rt : desc_.BlendState.RenderTarget) {
      rt.BlendEnable = FALSE;
      rt.LogicOpEnable = FALSE;
      rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    // Depth Stencil
    desc_.DepthStencilState.DepthEnable = FALSE;
    desc_.DepthStencilState.StencilEnable = FALSE;

    // Primitive Topology
    desc_.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // Sample Desc
    desc_.SampleDesc.Count = 1;
    desc_.SampleDesc.Quality = 0;
  }

  PipelineStateBuilder& SetRootSignature(ID3D12RootSignature* rootSig) {
    desc_.pRootSignature = rootSig;
    return *this;
  }

  PipelineStateBuilder& SetVertexShader(ID3DBlob* shader) {
    desc_.VS.pShaderBytecode = shader->GetBufferPointer();
    desc_.VS.BytecodeLength = shader->GetBufferSize();
    return *this;
  }

  PipelineStateBuilder& SetPixelShader(ID3DBlob* shader) {
    desc_.PS.pShaderBytecode = shader->GetBufferPointer();
    desc_.PS.BytecodeLength = shader->GetBufferSize();
    return *this;
  }

  PipelineStateBuilder& SetInputLayout(const std::vector<D3D12_INPUT_ELEMENT_DESC>& layout) {
    inputLayout_ = layout;
    desc_.InputLayout.pInputElementDescs = inputLayout_.data();
    desc_.InputLayout.NumElements = static_cast<UINT>(inputLayout_.size());
    return *this;
  }

  PipelineStateBuilder& SetRenderTargetFormat(DXGI_FORMAT format, UINT index = 0) {
    desc_.RTVFormats[index] = format;
    if (desc_.NumRenderTargets < (index + 1)) {
      desc_.NumRenderTargets = index + 1;
    }
    return *this;
  }

  PipelineStateBuilder& SetDepthStencilFormat(DXGI_FORMAT format) {
    desc_.DSVFormat = format;
    return *this;
  }

  PipelineStateBuilder& SetDepthTest(bool enable) {
    desc_.DepthStencilState.DepthEnable = enable;
    desc_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    return *this;
  }

  PipelineStateBuilder& SetDepthWrite(bool enable) {
    desc_.DepthStencilState.DepthWriteMask = enable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    return *this;
  }

  PipelineStateBuilder& EnableDepthTest(bool enable = true) {
    return this->SetDepthTest(enable).SetDepthWrite(enable);
  }

  PipelineStateBuilder& SetCullMode(D3D12_CULL_MODE cullMode) {
    desc_.RasterizerState.CullMode = cullMode;
    return *this;
  }

  PipelineStateBuilder& SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) {
    desc_.PrimitiveTopologyType = topology;
    return *this;
  }

  PipelineStateBuilder& SetBlendMode(BlendMode mode);

  ComPtr<ID3D12PipelineState> Build(ID3D12Device* device);

 private:
  D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_;
  std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout_;
};
