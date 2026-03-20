#include "Resource/Material/material_descriptor_pool.h"

#include <algorithm>
#include <string>

#include "Framework/Logging/logger.h"
#include "Framework/Render/render_settings.h"

bool MaterialDescriptorPool::Initialize(
  ID3D12Device* device, GetFenceValueFn get_fence_value, uint32_t frame_buffer_count, const MaterialDescriptorPoolConfig& config) {
  get_current_fence_value_ = std::move(get_fence_value);
  frame_buffer_count_ = frame_buffer_count;
  max_material_count_ = config.max_material_count;

  buffers_.resize(frame_buffer_count);
  for (uint32_t i = 0; i < frame_buffer_count; ++i) {
    std::wstring name = L"MaterialDescriptorPool_F" + std::to_wstring(i);
    if (!buffers_[i].Initialize(device, max_material_count_, name)) {
      Logger::LogFormat(
        LogLevel::Error, LogCategory::Graphic, Logger::Here(), "Failed to create MaterialDescriptorPool buffer frame {}", i);
      return false;
    }
  }

  slots_.reserve(256);

  // fallback
  MaterialDescriptor sentinel{};
  sentinel.albedo_texture_index = 0;
  sentinel.sampler_index = static_cast<uint32_t>(Rendering::SamplerType::AnisotropicWrap);
  sentinel.metallic_factor = 0.0f;
  sentinel.roughness_factor = 1.0f;
  Allocate(sentinel);

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Graphic,
    Logger::Here(),
    "MaterialDescriptorPool initialized (capacity: {}, frames: {})",
    max_material_count_,
    frame_buffer_count);
  return true;
}

MaterialHandle MaterialDescriptorPool::Allocate(const MaterialDescriptor& descriptor) {
  if (active_count_ >= max_material_count_) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Graphic, Logger::Here(), "MaterialDescriptorPool full ({}/{})", active_count_, max_material_count_);
    return MaterialHandle::Invalid();
  }

  uint32_t slot = AllocateSlot();
  slots_[slot].descriptor = descriptor;
  slots_[slot].occupied = true;

  for (uint32_t i = 0; i < frame_buffer_count_; ++i) {
    buffers_[i].UpdateAt(slot, descriptor);
  }

  ++active_count_;

  MaterialHandle handle;
  handle.index = slot;
  handle.generation = slots_[slot].generation;
  return handle;
}

void MaterialDescriptorPool::Update(MaterialHandle handle, const MaterialDescriptor& descriptor) {
  if (!IsValid(handle)) return;

  uint32_t slot = handle.index;
  slots_[slot].descriptor = descriptor;
  buffers_[current_frame_index_].UpdateAt(slot, descriptor);
  pending_sync_.push_back(slot);
}

void MaterialDescriptorPool::Free(MaterialHandle handle) {
  if (!IsValid(handle)) return;

  uint64_t fence_value = get_current_fence_value_ ? get_current_fence_value_() : 0;
  pending_frees_.push_back({handle.index, handle.generation, fence_value});
}

void MaterialDescriptorPool::SetCurrentFrame(uint32_t frame_index) {
  current_frame_index_ = frame_index;
}

void MaterialDescriptorPool::OnFrameBegin(uint32_t frame_index, uint64_t completed_fence_value) {
  current_frame_index_ = frame_index;

  for (uint32_t slot : pending_sync_) {
    if (slot < slots_.size() && slots_[slot].occupied) {
      buffers_[frame_index].UpdateAt(slot, slots_[slot].descriptor);
    }
  }
  pending_sync_.clear();

  auto it = std::remove_if(pending_frees_.begin(), pending_frees_.end(), [&](const PendingFree& pf) {
    if (pf.fence_value <= completed_fence_value) {
      if (pf.slot < slots_.size() && slots_[pf.slot].generation == pf.generation && slots_[pf.slot].occupied) {
        slots_[pf.slot].occupied = false;
        slots_[pf.slot].generation++;
        free_list_.push_back(pf.slot);
        --active_count_;
      }
      return true;
    }
    return false;
  });
  pending_frees_.erase(it, pending_frees_.end());
}

bool MaterialDescriptorPool::IsValid(MaterialHandle handle) const {
  if (!handle.IsValid()) return false;
  if (handle.index >= slots_.size()) return false;
  return slots_[handle.index].generation == handle.generation && slots_[handle.index].occupied;
}

D3D12_GPU_VIRTUAL_ADDRESS MaterialDescriptorPool::GetBufferAddress() const {
  return buffers_[current_frame_index_].GetGPUAddress();
}

MaterialDescriptorPool::Stats MaterialDescriptorPool::GetStats() const {
  return {
    .material_count = active_count_,
    .max_material_count = max_material_count_,
    .pending_frees = static_cast<uint32_t>(pending_frees_.size()),
    .pending_syncs = static_cast<uint32_t>(pending_sync_.size()),
  };
}

void MaterialDescriptorPool::Shutdown() {
  pending_frees_.clear();
  pending_sync_.clear();
  slots_.clear();
  free_list_.clear();
  buffers_.clear();
  active_count_ = 0;
}

uint32_t MaterialDescriptorPool::AllocateSlot() {
  if (!free_list_.empty()) {
    uint32_t slot = free_list_.back();
    free_list_.pop_back();
    return slot;
  }
  uint32_t slot = static_cast<uint32_t>(slots_.size());
  slots_.emplace_back();
  return slot;
}

void MaterialDescriptorPool::ReleaseSlot(uint32_t slot) {
  slots_[slot].occupied = false;
  slots_[slot].generation++;
  free_list_.push_back(slot);
  --active_count_;
}
