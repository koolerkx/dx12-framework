#include "debug_line_renderer.h"

#include "Frame/constant_buffers.h"
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

void DebugLineRenderer::AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color) {
  vertices_.push_back({start, color});
  vertices_.push_back({end, color});
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

  // Copy vertex data to upload buffer
  memcpy(alloc.cpu_ptr, vertices_.data(), data_size);

  current_frame_gpu_address_ = alloc.gpu_ptr;
  current_frame_vertex_count_ = static_cast<uint32_t>(vertices_.size());
}

void DebugLineRenderer::Render(const RenderFrameContext& frame, const Material* line_material, const DirectX::XMFLOAT4X4& view_proj) {
  if (vertices_.empty() || !line_material || !line_material->IsValid()) {
    return;
  }

  // Upload vertices to GPU
  UploadVertices(frame);

  if (current_frame_gpu_address_ == 0) {
    return;  // Upload failed
  }

  // Set pipeline state
  frame.command_list->SetPipelineState(line_material->GetPipelineState());
  frame.command_list->SetGraphicsRootSignature(line_material->GetRootSignature());

  // Bind Frame CBV (slot 0)
  frame.command_list->SetGraphicsRootConstantBufferView(0, frame.frame_cb->GetGPUAddress());

  // Bind Object CBV (slot 1) - identity world matrix
  ObjectCB obj_cb;
  DirectX::XMStoreFloat4x4(&obj_cb.world, DirectX::XMMatrixIdentity());
  DirectX::XMStoreFloat4x4(&obj_cb.worldViewProj, DirectX::XMLoadFloat4x4(&view_proj));
  obj_cb.color = DirectX::XMFLOAT4(1, 1, 1, 1);

  auto obj_alloc = frame.object_cb_allocator->Allocate(sizeof(ObjectCB));
  memcpy(obj_alloc.cpu_ptr, &obj_cb, sizeof(ObjectCB));
  frame.command_list->SetGraphicsRootConstantBufferView(1, obj_alloc.gpu_ptr);

  // Set vertex buffer
  D3D12_VERTEX_BUFFER_VIEW vbv = {};
  vbv.BufferLocation = current_frame_gpu_address_;
  vbv.SizeInBytes = current_frame_vertex_count_ * sizeof(LineVertex);
  vbv.StrideInBytes = sizeof(LineVertex);

  frame.command_list->IASetVertexBuffers(0, 1, &vbv);
  frame.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

  // Draw
  frame.command_list->DrawInstanced(current_frame_vertex_count_, 1, 0, 0);
}
