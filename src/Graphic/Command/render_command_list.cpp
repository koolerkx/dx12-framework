#include "render_command_list.h"

void RenderCommandList::DrawMeshInstanced(const Mesh* mesh, const std::vector<Graphics::Vertex::SpriteInstance>& instances) {
  if (!mesh || instances.empty()) {
    return;
  }

  // 1. Allocate GPU memory for instance data using DynamicUploadBuffer
  size_t data_size = instances.size() * sizeof(Graphics::Vertex::SpriteInstance);
  auto allocation = object_allocator_->Allocate(data_size);

  if (!allocation.cpu_ptr) {
    // Allocation failed, skip this draw call
    return;
  }

  // 2. Copy instance data to upload buffer (CPU -> GPU visible memory)
  memcpy(allocation.cpu_ptr, instances.data(), data_size);

  // 3. Create Instance Buffer View for Slot 1
  D3D12_VERTEX_BUFFER_VIEW instance_view{};
  instance_view.BufferLocation = allocation.gpu_ptr;
  instance_view.SizeInBytes = static_cast<UINT>(data_size);
  instance_view.StrideInBytes = sizeof(Graphics::Vertex::SpriteInstance);  // 96 bytes per instance

  // 4. Bind both vertex buffers: Slot 0 (mesh vertices) + Slot 1 (instance data)
  auto vbv = mesh->GetVertexBuffer().GetView();
  auto ibv = mesh->GetIndexBuffer().GetView();

  D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {vbv, instance_view};

  cmd_->IASetVertexBuffers(0, 2, vbvs);  // Bind both Slot 0 and Slot 1
  cmd_->IASetIndexBuffer(&ibv);
  cmd_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // 5. Execute DrawIndexedInstanced
  // Get index count from the first submesh (Quad has 6 indices for 2 triangles)
  uint32_t index_count = 6;  // Default fallback
  uint32_t start_index_location = 0;
  int32_t base_vertex_location = 0;

  if (mesh->GetSubMeshCount() > 0) {
    const auto& submesh = mesh->GetSubMesh(0);
    index_count = submesh.indexCount;
    start_index_location = submesh.startIndexLocation;
    base_vertex_location = submesh.baseVertexLocation;
  }

  cmd_->DrawIndexedInstanced(index_count,  // indexCountPerInstance
    static_cast<UINT>(instances.size()),   // instanceCount (number of glyphs/sprites)
    start_index_location,                  // startIndexLocation
    base_vertex_location,                  // baseVertexLocation
    0                                      // startInstanceLocation
  );
}
