#include "debug_line_renderer.h"

#include "Frame/constant_buffers.h"
#include "Framework/Core/color.h"
#include "Framework/Logging/logger.h"

bool DebugLineRenderer::Initialize(ID3D12Device* device) {
  device_ = device;
  return true;
}

void DebugLineRenderer::Shutdown() {
  vertices_.clear();
}

void DebugLineRenderer::Clear() {
  vertices_.clear();
  batches_.clear();
}

void DebugLineRenderer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color) {
  vertices_.push_back({.position = start, .color = color});
  vertices_.push_back({.position = end, .color = color});
}

void DebugLineRenderer::UploadVertices(const RenderFrameContext& frame) {
  batches_.clear();
  if (vertices_.empty()) return;

  constexpr size_t ALIGNMENT = 256;
  size_t page_size = 1024 * 1024;
  size_t max_vertices_per_batch = (page_size - ALIGNMENT) / sizeof(LineVertex);
  // round down to even number (each line = 2 vertices)
  max_vertices_per_batch &= ~1u;

  size_t offset = 0;
  while (offset < vertices_.size()) {
    size_t remaining = vertices_.size() - offset;
    size_t batch_count = (std::min)(remaining, max_vertices_per_batch);
    size_t data_size = batch_count * sizeof(LineVertex);

    auto alloc = frame.object_cb_allocator->Allocate(data_size);
    if (alloc.cpu_ptr == nullptr) {
      Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DebugLineRenderer] Failed to allocate upload buffer");
      return;
    }

    memcpy(alloc.cpu_ptr, vertices_.data() + offset, data_size);
    batches_.push_back({.gpu_address = alloc.gpu_ptr, .vertex_count = static_cast<uint32_t>(batch_count)});
    offset += batch_count;
  }
}

void DebugLineRenderer::Render(const RenderFrameContext& frame, const Material* line_material, const Matrix4& view_proj) {
  if (vertices_.empty() || !line_material || !line_material->IsValid()) {
    return;
  }

  UploadVertices(frame);

  if (batches_.empty()) return;

  frame.command_list->SetPipelineState(line_material->GetPipelineState());
  frame.command_list->SetGraphicsRootSignature(line_material->GetRootSignature());

  FrameCB frame_cb = {};
  frame_cb.viewProj = view_proj;
  auto cb_alloc = frame.object_cb_allocator->Allocate<FrameCB>();
  memcpy(cb_alloc.cpu_ptr, &frame_cb, sizeof(FrameCB));
  frame.command_list->SetGraphicsRootConstantBufferView(0, cb_alloc.gpu_ptr);

  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

  for (const auto& batch : batches_) {
    D3D12_VERTEX_BUFFER_VIEW vbv = {};
    vbv.BufferLocation = batch.gpu_address;
    vbv.SizeInBytes = batch.vertex_count * sizeof(LineVertex);
    vbv.StrideInBytes = sizeof(LineVertex);

    frame.command_list->IASetVertexBuffers(0, 1, &vbv);
    frame.command_list->DrawInstanced(batch.vertex_count, 1, 0, 0);
  }
}
