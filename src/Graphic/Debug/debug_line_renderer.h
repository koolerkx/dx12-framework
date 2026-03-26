#pragma once
#include <d3d12.h>

#include <span>
#include <vector>

#include "Frame/render_frame_context.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/render_service.h"
#include "Pipeline/material.h"
#include "Pipeline/vertex_types.h"
#include "Resource/Buffer/structured_buffer.h"

using Math::Matrix4;
using Math::Vector3;
using Math::Vector4;

using LineVertex = Graphics::Vertex::LineVertex;

class DebugLineRenderer {
 public:
  bool Initialize(ID3D12Device* device);
  void Shutdown();

  void AddLine(const Vector3& start, const Vector3& end, const Vector4& color);
  void AddLine(
    LineSetId id, const Vector3& position, const Vector3& axis_x, const Vector3& axis_y, const Vector3& axis_z, const Vector4& color);

  void Clear();

  void Render(const RenderFrameContext& frame, const Material* instanced_material, const Matrix4& view_proj);

  bool HasLines() const;
  size_t GetLineCount() const;

 private:
  struct LineInstance {
    Vector3 position;
    Vector3 axis_x;
    Vector3 axis_y;
    Vector3 axis_z;
    Vector4 color;
  };
  static_assert(sizeof(LineInstance) == 64);

  struct LineSetInfo {
    uint32_t vertex_offset;
    uint32_t vertex_count;
  };

  void RegisterLineSet(LineSetId id, std::span<const LineVertex> vertices);
  void BuildResources();

  static constexpr uint32_t LINE_SET_COUNT = static_cast<uint32_t>(LineSetId::COUNT);
  static constexpr uint32_t MAX_INSTANCES = 65536;

  ID3D12Device* device_ = nullptr;

  // Line set registration
  LineSetInfo line_sets_[LINE_SET_COUNT]{};
  std::vector<LineVertex> staging_vertices_;
  ComPtr<ID3D12Resource> line_set_vb_;
  D3D12_VERTEX_BUFFER_VIEW line_set_vbv_{};

  // Per-frame instances
  Graphics::StructuredBuffer<LineInstance> instance_buffer_;
  std::vector<LineInstance> pending_instances_[LINE_SET_COUNT];
  size_t last_frame_instance_counts_[LINE_SET_COUNT]{};

  // Command signature for non-indexed instanced draw
  ComPtr<ID3D12CommandSignature> draw_command_signature_;
};
