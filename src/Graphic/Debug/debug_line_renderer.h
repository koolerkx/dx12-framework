#pragma once
#include <d3d12.h>

#include <span>
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

  // Reserve space for multiple lines at once, returns span of vertex pairs (2 vertices per line)
  std::span<LineVertex> ReserveLines(uint32_t line_count);

  void Clear();

  void Render(const RenderFrameContext& frame, const Material* line_material, const Matrix4& view_proj);

  bool HasLines() const {
    return !vertices_.empty();
  }
  size_t GetLineCount() const {
    return vertices_.size() / 2;
  }

 private:
  struct Batch {
    D3D12_GPU_VIRTUAL_ADDRESS gpu_address;
    uint32_t vertex_count;
  };

  void UploadVertices(const RenderFrameContext& frame);

  ID3D12Device* device_ = nullptr;
  std::vector<LineVertex> vertices_;
  std::vector<Batch> batches_;
  size_t last_frame_vertex_count_ = 0;
};
