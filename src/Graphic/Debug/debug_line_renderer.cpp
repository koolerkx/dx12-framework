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
  current_frame_gpu_address_ = 0;
  current_frame_vertex_count_ = 0;
}

void DebugLineRenderer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color) {
  vertices_.push_back({.position = start, .color = color});
  vertices_.push_back({.position = end, .color = color});
}

void DebugLineRenderer::UploadVertices(const RenderFrameContext& frame) {
  if (vertices_.empty()) {
    current_frame_gpu_address_ = 0;
    current_frame_vertex_count_ = 0;
    return;
  }

  size_t data_size = vertices_.size() * sizeof(LineVertex);

  DynamicUploadBuffer::Allocation alloc = frame.object_cb_allocator->Allocate(data_size);

  if (alloc.cpu_ptr == nullptr) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(), "[DebugLineRenderer] Failed to allocate upload buffer");
    current_frame_gpu_address_ = 0;
    current_frame_vertex_count_ = 0;
    return;
  }

  memcpy(alloc.cpu_ptr, vertices_.data(), data_size);

  current_frame_gpu_address_ = alloc.gpu_ptr;
  current_frame_vertex_count_ = static_cast<uint32_t>(vertices_.size());
}

void DebugLineRenderer::Render(const RenderFrameContext& frame, const Material* line_material, const Matrix4& view_proj) {
  if (vertices_.empty() || !line_material || !line_material->IsValid()) {
    return;
  }

  UploadVertices(frame);

  if (current_frame_gpu_address_ == 0) {
    return;
  }

  frame.command_list->SetPipelineState(line_material->GetPipelineState());
  frame.command_list->SetGraphicsRootSignature(line_material->GetRootSignature());

  frame.command_list->SetGraphicsRootConstantBufferView(0, frame.frame_cb->GetGPUAddress());

  ObjectCB obj_cb;
  obj_cb.world = Matrix4::Identity;
  obj_cb.worldViewProj = view_proj;
  obj_cb.color = colors::White;

  auto obj_alloc = frame.object_cb_allocator->Allocate(sizeof(ObjectCB));
  memcpy(obj_alloc.cpu_ptr, &obj_cb, sizeof(ObjectCB));
  frame.command_list->SetGraphicsRootConstantBufferView(1, obj_alloc.gpu_ptr);

  D3D12_VERTEX_BUFFER_VIEW vbv = {};
  vbv.BufferLocation = current_frame_gpu_address_;
  vbv.SizeInBytes = current_frame_vertex_count_ * sizeof(LineVertex);
  vbv.StrideInBytes = sizeof(LineVertex);

  frame.command_list->IASetVertexBuffers(0, 1, &vbv);
  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

  frame.command_list->DrawInstanced(current_frame_vertex_count_, 1, 0, 0);
}
