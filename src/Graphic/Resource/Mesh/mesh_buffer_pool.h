#pragma once

#include <d3d12.h>

#include <cstdint>
#include <functional>
#include <span>
#include <vector>

#include "Core/types.h"
#include "Framework/Render/render_handles.h"
#include "Resource/Allocator/free_block_allocator.h"
#include "Pipeline/vertex_types.h"
#include "Resource/Mesh/mesh_descriptor.h"

using Graphics::Vertex::ModelVertex;
using Graphics::Vertex::SpriteVertex;

enum class VertexLayout : uint8_t { Model = 0, Sprite = 1 };

struct MeshAllocation {
  MeshHandle handle;
  bool success = false;
};

struct MeshBufferPoolConfig {
  uint32_t initial_vertex_capacity = 1'000'000;
  uint32_t initial_index_capacity = 3'000'000;
  uint32_t sprite_vertex_capacity = 100'000;
  uint32_t max_mesh_count = 4096;
};

class MeshBufferPool {
 public:
  using ExecuteSyncFn = std::function<void(std::function<void(ID3D12GraphicsCommandList*)>)>;
  using GetFenceValueFn = std::function<uint64_t()>;

  struct CreateInfo {
    ID3D12Device* device = nullptr;
    ExecuteSyncFn execute_sync;
    GetFenceValueFn get_current_fence_value;
  };

  bool Initialize(const CreateInfo& info, const MeshBufferPoolConfig& config = {});

  MeshAllocation Allocate(std::span<const ModelVertex> vertices, std::span<const uint32_t> indices);
  MeshAllocation Allocate(std::span<const SpriteVertex> vertices, std::span<const uint32_t> indices);
  void Free(MeshHandle handle);

  void ProcessDeferredFrees(uint64_t completed_fence_value);
  void Shutdown();

  D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const;
  D3D12_VERTEX_BUFFER_VIEW GetSpriteVertexBufferView() const;
  D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const;
  D3D12_GPU_VIRTUAL_ADDRESS GetDescriptorBufferAddress() const;

  const MeshDescriptor* GetDescriptor(MeshHandle handle) const;
  VertexLayout GetVertexLayout(MeshHandle handle) const;
  bool IsValid(MeshHandle handle) const;

  struct Stats {
    uint32_t vertex_used = 0;
    uint32_t vertex_total = 0;
    uint32_t index_used = 0;
    uint32_t index_total = 0;
    uint32_t mesh_count = 0;
    uint32_t max_mesh_count = 0;
    uint32_t pending_frees = 0;
  };
  Stats GetStats() const;

 private:
  struct SlotData {
    MeshDescriptor descriptor;
    uint32_t generation = 0;
    VertexLayout layout = VertexLayout::Model;
    bool occupied = false;
  };

  struct PendingFree {
    uint32_t slot;
    uint32_t generation;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t index_offset;
    uint32_t index_count;
    uint64_t fence_value;
    VertexLayout layout = VertexLayout::Model;
  };

  void UploadRegion(ID3D12Resource* dest, uint64_t dest_offset, const void* data, uint64_t size);
  uint32_t AllocateSlot();
  void ReleaseSlot(uint32_t slot);
  void UpdateDescriptorOnGpu(uint32_t slot);

  ID3D12Device* device_ = nullptr;
  ExecuteSyncFn execute_sync_;
  GetFenceValueFn get_current_fence_value_;

  ComPtr<ID3D12Resource> vertex_buffer_;
  ComPtr<ID3D12Resource> sprite_vertex_buffer_;
  ComPtr<ID3D12Resource> index_buffer_;
  ComPtr<ID3D12Resource> descriptor_buffer_;

  FreeBlockAllocator vertex_allocator_;
  FreeBlockAllocator sprite_vertex_allocator_;
  FreeBlockAllocator index_allocator_;

  uint32_t vertex_capacity_ = 0;
  uint32_t sprite_vertex_capacity_ = 0;
  uint32_t index_capacity_ = 0;
  uint32_t max_mesh_count_ = 0;

  std::vector<SlotData> slots_;
  std::vector<uint32_t> free_slots_;
  std::vector<PendingFree> pending_frees_;
};
