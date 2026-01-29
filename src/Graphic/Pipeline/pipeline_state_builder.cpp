#include "pipeline_state_builder.h"

#include <stdexcept>

ComPtr<ID3D12PipelineState> PipelineStateBuilder::Build(ID3D12Device* device) {
  ComPtr<ID3D12PipelineState> pso;
  HRESULT hr = device->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(&pso));
  if (FAILED(hr)) {
    throw std::runtime_error("Failed to create graphics pipeline state");
  }
  return pso;
}

PipelineStateBuilder& PipelineStateBuilder::SetBlendMode(BlendMode mode) {
  D3D12_RENDER_TARGET_BLEND_DESC& blendDesc = desc_.BlendState.RenderTarget[0];

  // reset all
  blendDesc.LogicOpEnable = false;
  blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
  blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
  blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

  switch (mode) {
    case BlendMode::Opaque:
      blendDesc.BlendEnable = false;
      break;
    case BlendMode::AlphaBlend:
      // RGB = src.rgb * src.a + dst.rgb * (1 - src.a)
      // A = src.a + dst.a * (1 - src.a)
      blendDesc.BlendEnable = TRUE;
      blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
      blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
      blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
      blendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
      break;
    case BlendMode::Premultiplied:
      // RGB = src.rgb + dst.rgb * (1 - src.a)
      blendDesc.BlendEnable = TRUE;
      blendDesc.SrcBlend = D3D12_BLEND_ONE;
      blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
      blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
      blendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
      break;
    case BlendMode::Additive:
      // RGB = src.rgb * src.a + dst.rgb * 1
      // A = src.a + dst.a * (1 - src.a)
      blendDesc.BlendEnable = TRUE;
      blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
      blendDesc.DestBlend = D3D12_BLEND_ONE;
      blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
      blendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
      break;
    case BlendMode::NonPremultiplied:
      blendDesc.BlendEnable = TRUE;
      blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
      blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
      blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
      blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
      break;
    default:
      break;
  }
  return *this;
}
