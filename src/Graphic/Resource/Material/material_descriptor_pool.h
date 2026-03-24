#pragma once

#include <d3d12.h>

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include "Framework/Render/material_descriptor.h"
#include "Framework/Render/render_handles.h"
#include "Resource/Buffer/structured_buffer.h"

struct MaterialDescriptorPoolConfig {
  uint32_t max_material_count = 4096;
};

class MaterialDescriptorPool {
 public:
  static constexpr uint32_t SENTINEL_INDEX = 0;

  using GetFenceValueFn = std::function<uint64_t()>;

  bool Initialize(
    ID3D12Device* device, GetFenceValueFn get_fence_value, uint32_t frame_buffer_count, const MaterialDescriptorPoolConfig& config = {});

  MaterialHandle Allocate(const MaterialDescriptor& descriptor);
  MaterialHandle Update(MaterialHandle handle, const MaterialDescriptor& descriptor);
  void Free(MaterialHandle handle);

  void SetCurrentFrame(uint32_t frame_index);
  void OnFrameBegin(uint32_t frame_index, uint64_t completed_fence_value);

  bool IsValid(MaterialHandle handle) const;
  D3D12_GPU_VIRTUAL_ADDRESS GetBufferAddress() const;

  struct Stats {
    uint32_t material_count = 0;
    uint32_t max_material_count = 0;
    uint32_t pending_frees = 0;
    uint32_t pending_syncs = 0;
  };
  Stats GetStats() const;

  void Shutdown();

 private:
  struct Slot {
    MaterialDescriptor descriptor{};
    uint32_t generation = 0;
    uint32_t ref_count = 0;
    bool occupied = false;
  };

  struct PendingFree {
    uint32_t slot;
    uint32_t generation;
    uint64_t fence_value;
  };

  uint32_t AllocateSlot();
  void ReleaseSlot(uint32_t slot);

  GetFenceValueFn get_current_fence_value_;
  uint32_t current_frame_index_ = 0;
  uint32_t frame_buffer_count_ = 2;

  std::vector<Graphics::StructuredBuffer<MaterialDescriptor>> buffers_;

  std::vector<Slot> slots_;
  std::vector<uint32_t> free_list_;
  std::vector<PendingFree> pending_frees_;
  std::vector<uint32_t> pending_sync_;

  uint32_t max_material_count_ = 0;
  uint32_t active_count_ = 0;

  std::unordered_map<MaterialDescriptor, uint32_t, MaterialDescriptorHash> dedup_map_;
};
