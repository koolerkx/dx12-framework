#include "pipeline_state_builder.h"

#include "Framework/Logging/logger.h"

ComPtr<ID3D12PipelineState> PipelineStateBuilder::Build(ID3D12Device* device) {
  if (!inputLayout_.empty()) {  // prevent chaining split at middle and pointing to a temporary object
    desc_.InputLayout.pInputElementDescs = inputLayout_.data();
    desc_.InputLayout.NumElements = static_cast<UINT>(inputLayout_.size());
  }

  ComPtr<ID3D12PipelineState> pso;
  HRESULT hr = device->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(&pso));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Graphic,
      Logger::Here(),
      "[PipelineStateBuilder] Failed to create PSO, HRESULT=0x{:08X}",
      static_cast<uint32_t>(hr));
    return nullptr;
  }
  if (name_) {
    pso->SetName(name_);
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
