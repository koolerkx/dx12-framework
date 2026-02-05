#pragma once
#include <d3d12.h>

#include <vector>

#include "Frame/render_frame_context.h"
#include "Framework/Math/Math.h"
#include "Pipeline/material.h"
#include "Pipeline/vertex_types.h"

using Math::Matrix4;
using Math::Vector3;
using Math::Vector4;

using LineVertex = Graphics::Vertex::LineVertex;

class DebugLineRenderer {
 public:
  bool Initialize(ID3D12Device* device);
  void Shutdown();

  void AddLine(const Vector3& start, const Vector3& end, const Vector4& color);

  void Clear();

  void Render(const RenderFrameContext& frame, const Material* line_material, const Matrix4& view_proj);

  bool HasLines() const {
    return !vertices_.empty();
  }
  size_t GetLineCount() const {
    return vertices_.size() / 2;
  }

 private:
  void UploadVertices(const RenderFrameContext& frame);

  ID3D12Device* device_ = nullptr;
  std::vector<LineVertex> vertices_;

  D3D12_GPU_VIRTUAL_ADDRESS current_frame_gpu_address_ = 0;
  uint32_t current_frame_vertex_count_ = 0;
};
