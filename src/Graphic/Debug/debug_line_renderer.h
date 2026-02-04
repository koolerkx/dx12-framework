#pragma once
#include <DirectXMath.h>
#include <d3d12.h>

#include <vector>

#include "Frame/render_frame_context.h"
#include "Pipeline/material.h"
#include "Pipeline/vertex_types.h"

using LineVertex = Graphics::Vertex::LineVertex;

class DebugLineRenderer {
 public:
  bool Initialize(ID3D12Device* device);
  void Shutdown();

  // Basic API - add a single line
  void AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color);

  // Clear accumulated lines (called at beginning of frame)
  void Clear();

  // Upload and render all accumulated lines
  void Render(const RenderFrameContext& frame, const Material* line_material, const DirectX::XMFLOAT4X4& view_proj);

  // Check if any lines are pending
  bool HasLines() const {
    return !vertices_.empty();
  }
  size_t GetLineCount() const {
    return vertices_.size() / 2;
  }

 private:
  void UploadVertices(const RenderFrameContext& frame);

  ID3D12Device* device_ = nullptr;
  std::vector<LineVertex> vertices_;  // CPU-side accumulation

  // GPU upload address (valid for current frame only)
  D3D12_GPU_VIRTUAL_ADDRESS current_frame_gpu_address_ = 0;
  uint32_t current_frame_vertex_count_ = 0;
};
