#include "debug_line_renderer.h"

#include "Frame/constant_buffers.h"
#include "Framework/Logging/logger.h"
#include "Framework/Math/unit_circle_lut.h"
#include "Pipeline/root_parameter_slots.h"

namespace {

Vector4 WHITE(1.0f, 1.0f, 1.0f, 1.0f);

template <int Segments>
std::vector<LineVertex> BuildUnitSphereWireframe() {
  const auto& lut = Math::GetCircleLUT<Segments>();
  std::vector<LineVertex> vertices;
  vertices.reserve(Segments * 3 * 2);

  for (int i = 0; i < Segments; ++i) {
    int next = (i + 1) % Segments;
    float c1 = lut.points[i].cos, s1 = lut.points[i].sin;
    float c2 = lut.points[next].cos, s2 = lut.points[next].sin;

    vertices.push_back({.position = {c1, s1, 0.0f}, .color = WHITE});
    vertices.push_back({.position = {c2, s2, 0.0f}, .color = WHITE});
    vertices.push_back({.position = {c1, 0.0f, s1}, .color = WHITE});
    vertices.push_back({.position = {c2, 0.0f, s2}, .color = WHITE});
    vertices.push_back({.position = {0.0f, c1, s1}, .color = WHITE});
    vertices.push_back({.position = {0.0f, c2, s2}, .color = WHITE});
  }
  return vertices;
}

template <int Segments>
std::vector<LineVertex> BuildUnitCircle() {
  const auto& lut = Math::GetCircleLUT<Segments>();
  std::vector<LineVertex> vertices;
  vertices.reserve(Segments * 2);

  for (int i = 0; i < Segments; ++i) {
    int next = (i + 1) % Segments;
    vertices.push_back({.position = {lut.points[i].cos, lut.points[i].sin, 0.0f}, .color = WHITE});
    vertices.push_back({.position = {lut.points[next].cos, lut.points[next].sin, 0.0f}, .color = WHITE});
  }
  return vertices;
}

std::vector<LineVertex> BuildUnitCubeWireframe() {
  // Unit cube from (-1,-1,-1) to (1,1,1)
  Vector3 c[8] = {
    {-1, -1, -1},
    {1, -1, -1},
    {1, -1, 1},
    {-1, -1, 1},
    {-1, 1, -1},
    {1, 1, -1},
    {1, 1, 1},
    {-1, 1, 1},
  };

  constexpr int EDGES[][2] = {
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0},
    {4, 5},
    {5, 6},
    {6, 7},
    {7, 4},
    {0, 4},
    {1, 5},
    {2, 6},
    {3, 7},
  };

  std::vector<LineVertex> vertices;
  vertices.reserve(24);
  for (auto [a, b] : EDGES) {
    vertices.push_back({.position = c[a], .color = WHITE});
    vertices.push_back({.position = c[b], .color = WHITE});
  }
  return vertices;
}

std::vector<LineVertex> BuildUnitRect() {
  // Unit rect in XZ plane from (-1,0,-1) to (1,0,1)
  Vector3 c[4] = {{-1, 0, -1}, {1, 0, -1}, {1, 0, 1}, {-1, 0, 1}};
  std::vector<LineVertex> vertices;
  vertices.reserve(8);
  for (int i = 0; i < 4; ++i) {
    vertices.push_back({.position = c[i], .color = WHITE});
    vertices.push_back({.position = c[(i + 1) % 4], .color = WHITE});
  }
  return vertices;
}

}  // namespace

bool DebugLineRenderer::Initialize(ID3D12Device* device) {
  device_ = device;

  // Unit line: (0,0,0) → (1,0,0)
  LineVertex unit_line[2] = {
    {.position = {0, 0, 0}, .color = WHITE},
    {.position = {1, 0, 0}, .color = WHITE},
  };
  RegisterLineSet(LineSetId::UnitLine, unit_line);
  RegisterLineSet(LineSetId::Sphere8, BuildUnitSphereWireframe<8>());
  RegisterLineSet(LineSetId::Sphere16, BuildUnitSphereWireframe<16>());
  RegisterLineSet(LineSetId::Sphere32, BuildUnitSphereWireframe<32>());
  RegisterLineSet(LineSetId::Circle32, BuildUnitCircle<32>());
  RegisterLineSet(LineSetId::UnitCube, BuildUnitCubeWireframe());
  RegisterLineSet(LineSetId::UnitRect, BuildUnitRect());

  BuildResources();
  return true;
}

void DebugLineRenderer::Shutdown() {
  instance_buffer_ = {};
  line_set_vb_.Reset();
  draw_command_signature_.Reset();
}

void DebugLineRenderer::Clear() {
  for (uint32_t i = 0; i < LINE_SET_COUNT; ++i) {
    last_frame_instance_counts_[i] = pending_instances_[i].size();
    pending_instances_[i].clear();
    pending_instances_[i].reserve(last_frame_instance_counts_[i]);
  }
}

void DebugLineRenderer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color) {
  pending_instances_[static_cast<uint32_t>(LineSetId::UnitLine)].push_back({
    .position = start,
    .axis_x = end - start,
    .axis_y = {},
    .axis_z = {},
    .color = color,
  });
}

void DebugLineRenderer::AddLine(
  LineSetId id, const Vector3& position, const Vector3& axis_x, const Vector3& axis_y, const Vector3& axis_z, const Vector4& color) {
  pending_instances_[static_cast<uint32_t>(id)].push_back({
    .position = position,
    .axis_x = axis_x,
    .axis_y = axis_y,
    .axis_z = axis_z,
    .color = color,
  });
}

bool DebugLineRenderer::HasLines() const {
  for (uint32_t i = 0; i < LINE_SET_COUNT; ++i) {
    if (!pending_instances_[i].empty()) return true;
  }
  return false;
}

size_t DebugLineRenderer::GetLineCount() const {
  size_t count = 0;
  for (uint32_t i = 0; i < LINE_SET_COUNT; ++i) {
    count += pending_instances_[i].size() * (line_sets_[i].vertex_count / 2);
  }
  return count;
}

void DebugLineRenderer::RegisterLineSet(LineSetId id, std::span<const LineVertex> vertices) {
  uint32_t idx = static_cast<uint32_t>(id);
  uint32_t offset = static_cast<uint32_t>(staging_vertices_.size());
  staging_vertices_.insert(staging_vertices_.end(), vertices.begin(), vertices.end());
  line_sets_[idx] = {offset, static_cast<uint32_t>(vertices.size())};
}

void DebugLineRenderer::BuildResources() {
  if (staging_vertices_.empty() || !device_) return;

  // Static VB
  size_t vb_size = staging_vertices_.size() * sizeof(LineVertex);
  D3D12_HEAP_PROPERTIES heap_props = {.Type = D3D12_HEAP_TYPE_UPLOAD};
  D3D12_RESOURCE_DESC desc = {
    .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
    .Width = vb_size,
    .Height = 1,
    .DepthOrArraySize = 1,
    .MipLevels = 1,
    .SampleDesc = {1, 0},
    .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
  };

  HRESULT hr = device_->CreateCommittedResource(
    &heap_props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&line_set_vb_));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DebugLineRenderer] Failed to create line set VB");
    return;
  }

  void* mapped = nullptr;
  D3D12_RANGE read_range = {0, 0};
  line_set_vb_->Map(0, &read_range, &mapped);
  memcpy(mapped, staging_vertices_.data(), vb_size);
  line_set_vb_->Unmap(0, nullptr);

  line_set_vbv_ = {
    .BufferLocation = line_set_vb_->GetGPUVirtualAddress(),
    .SizeInBytes = static_cast<UINT>(vb_size),
    .StrideInBytes = sizeof(LineVertex),
  };
  staging_vertices_.clear();

  // Instance StructuredBuffer
  instance_buffer_.Initialize(device_, MAX_INSTANCES, L"DebugLineInstanceBuffer");

  // Command signature for non-indexed instanced draw
  D3D12_INDIRECT_ARGUMENT_DESC arg_desc = {.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW};
  D3D12_COMMAND_SIGNATURE_DESC sig_desc = {
    .ByteStride = sizeof(D3D12_DRAW_ARGUMENTS),
    .NumArgumentDescs = 1,
    .pArgumentDescs = &arg_desc,
  };
  hr = device_->CreateCommandSignature(&sig_desc, nullptr, IID_PPV_ARGS(&draw_command_signature_));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DebugLineRenderer] Failed to create draw command signature");
  }
}

void DebugLineRenderer::Render(const RenderFrameContext& frame, const Material* material, const Matrix4& view_proj) {
  if (!material || !material->IsValid() || !draw_command_signature_ || !HasLines()) return;

  // Concatenate all instances into StructuredBuffer, track per-set offsets
  struct DrawGroup {
    uint32_t vertex_offset, vertex_count, instance_offset, instance_count;
  };

  std::vector<LineInstance> combined;
  std::vector<DrawGroup> groups;
  uint32_t instance_offset = 0;
  uint32_t remaining = MAX_INSTANCES;

  for (uint32_t i = 0; i < LINE_SET_COUNT && remaining > 0; ++i) {
    if (pending_instances_[i].empty()) continue;

    uint32_t count = static_cast<uint32_t>(pending_instances_[i].size());
    if (count > remaining) {
      Logger::LogFormat(LogLevel::Warn,
        LogCategory::Graphic,
        Logger::Here(),
        "[DebugLineRenderer] Capping line set {} from {} to {} instances",
        i,
        count,
        remaining);
      count = remaining;
    }

    groups.push_back({line_sets_[i].vertex_offset, line_sets_[i].vertex_count, instance_offset, count});
    combined.insert(combined.end(), pending_instances_[i].begin(), pending_instances_[i].begin() + count);
    instance_offset += count;
    remaining -= count;
  }

  if (groups.empty()) return;

  uint32_t total_instances = instance_offset;

  // Upload instance data to StructuredBuffer
  instance_buffer_.Update(combined.data(), total_instances);

  // Build unified instance index buffer [0, 1, 2, ..., N-1] — same pattern as objectIndex buffer
  auto index_alloc = frame.object_cb_allocator->Allocate(total_instances * sizeof(uint32_t));
  if (!index_alloc.cpu_ptr) return;
  auto* indices = static_cast<uint32_t*>(index_alloc.cpu_ptr);
  for (uint32_t i = 0; i < total_instances; ++i) {
    indices[i] = i;
  }

  // Bind IA slot 1 — per-instance index stream (OBJECT_INDEX semantic, step rate 1)
  D3D12_VERTEX_BUFFER_VIEW index_vbv = {
    .BufferLocation = index_alloc.gpu_ptr,
    .SizeInBytes = static_cast<UINT>(total_instances * sizeof(uint32_t)),
    .StrideInBytes = sizeof(uint32_t),
  };

  // Set pipeline state
  frame.command_list->SetPipelineState(material->GetPipelineState());
  frame.command_list->SetGraphicsRootSignature(material->GetRootSignature());

  FrameCB frame_cb = {};
  frame_cb.viewProj = view_proj;
  auto cb_alloc = frame.object_cb_allocator->Allocate<FrameCB>();
  memcpy(cb_alloc.cpu_ptr, &frame_cb, sizeof(FrameCB));
  frame.command_list->SetGraphicsRootConstantBufferView(0, cb_alloc.gpu_ptr);

  frame.command_list->SetGraphicsRootShaderResourceView(
    RootSlot::ToIndex(RootSlot::ShaderResource::ObjectBuffer), instance_buffer_.GetGPUAddress());

  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
  frame.command_list->IASetVertexBuffers(0, 1, &line_set_vbv_);
  frame.command_list->IASetVertexBuffers(1, 1, &index_vbv);

  // Build indirect draw args — StartInstanceLocation offsets IA slot 1 read
  size_t args_size = groups.size() * sizeof(D3D12_DRAW_ARGUMENTS);
  auto args_alloc = frame.object_cb_allocator->Allocate(args_size);
  if (!args_alloc.cpu_ptr) return;

  auto* args = static_cast<D3D12_DRAW_ARGUMENTS*>(args_alloc.cpu_ptr);
  for (size_t i = 0; i < groups.size(); ++i) {
    args[i] = {
      .VertexCountPerInstance = groups[i].vertex_count,
      .InstanceCount = groups[i].instance_count,
      .StartVertexLocation = groups[i].vertex_offset,
      .StartInstanceLocation = groups[i].instance_offset,
    };
  }

  frame.command_list->ExecuteIndirect(
    draw_command_signature_.Get(), static_cast<UINT>(groups.size()), args_alloc.resource, args_alloc.offset_in_resource, nullptr, 0);
}
