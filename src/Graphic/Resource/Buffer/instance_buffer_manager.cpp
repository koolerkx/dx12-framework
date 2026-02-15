#include "Resource/Buffer/instance_buffer_manager.h"

#include <algorithm>
#include <string>

#include "Framework/Logging/logger.h"

bool InstanceBufferManager::Initialize(ID3D12Device* device) {
  device_ = device;
  return device_ != nullptr;
}

InstanceBufferHandle InstanceBufferManager::Create(uint32_t capacity) {
  if (!device_ || capacity == 0) return InstanceBufferHandle::Invalid;

  uint32_t slot = AllocateSlot();

  Entry entry;
  std::wstring name = L"InstanceBuffer_" + std::to_wstring(slot);
  if (!entry.buffer.Initialize(device_, capacity, name)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Graphic, Logger::Here(),
      "Failed to create instance buffer with capacity {}", capacity);
    ReleaseSlot(slot);
    return InstanceBufferHandle::Invalid;
  }

  slots_[slot].entry = std::move(entry);
  return EncodeHandle(slot, slots_[slot].generation);
}

void InstanceBufferManager::Update(InstanceBufferHandle handle, const GPUInstanceData* data, uint32_t count) {
  uint32_t slot;
  if (!DecodeAndValidate(handle, slot)) return;

  auto& entry = *slots_[slot].entry;
  entry.buffer.Update(data, count);
  entry.count = count;
}

D3D12_GPU_VIRTUAL_ADDRESS InstanceBufferManager::GetAddress(InstanceBufferHandle handle) const {
  uint32_t slot;
  if (!DecodeAndValidate(handle, slot)) return 0;

  return slots_[slot].entry->buffer.GetGPUAddress();
}

uint32_t InstanceBufferManager::GetCount(InstanceBufferHandle handle) const {
  uint32_t slot;
  if (!DecodeAndValidate(handle, slot)) return 0;

  return slots_[slot].entry->count;
}

void InstanceBufferManager::Destroy(InstanceBufferHandle handle, uint64_t fence_value) {
  uint32_t slot;
  if (!DecodeAndValidate(handle, slot)) return;

  pending_frees_.push_back({slot, slots_[slot].generation, fence_value});
}

void InstanceBufferManager::ProcessDeferredFrees(uint64_t completed_fence_value) {
  auto it = std::remove_if(pending_frees_.begin(), pending_frees_.end(),
    [&](const PendingFree& pf) {
      if (pf.fence_value <= completed_fence_value) {
        if (pf.slot < slots_.size() && slots_[pf.slot].generation == pf.generation) {
          slots_[pf.slot].entry.reset();
          slots_[pf.slot].generation++;
          free_list_.push_back(pf.slot);
        }
        return true;
      }
      return false;
    });
  pending_frees_.erase(it, pending_frees_.end());
}

void InstanceBufferManager::FlushAllPending() {
  for (auto& pf : pending_frees_) {
    if (pf.slot < slots_.size() && slots_[pf.slot].generation == pf.generation) {
      slots_[pf.slot].entry.reset();
      slots_[pf.slot].generation++;
      free_list_.push_back(pf.slot);
    }
  }
  pending_frees_.clear();
}

void InstanceBufferManager::Shutdown() {
  pending_frees_.clear();
  slots_.clear();
  free_list_.clear();
  device_ = nullptr;
}

InstanceBufferHandle InstanceBufferManager::EncodeHandle(uint32_t slot, uint16_t generation) {
  return static_cast<InstanceBufferHandle>((static_cast<uint32_t>(generation) << SLOT_BITS) | (slot + 1));
}

bool InstanceBufferManager::DecodeAndValidate(InstanceBufferHandle handle, uint32_t& out_slot) const {
  uint32_t raw = static_cast<uint32_t>(handle);
  if (raw == 0) return false;

  uint32_t slot = (raw & SLOT_MASK) - 1;
  uint16_t generation = static_cast<uint16_t>(raw >> SLOT_BITS);

  if (slot >= slots_.size()) return false;
  if (slots_[slot].generation != generation) return false;
  if (!slots_[slot].entry) return false;

  out_slot = slot;
  return true;
}

uint32_t InstanceBufferManager::AllocateSlot() {
  if (!free_list_.empty()) {
    uint32_t slot = free_list_.back();
    free_list_.pop_back();
    return slot;
  }

  uint32_t slot = static_cast<uint32_t>(slots_.size());
  slots_.emplace_back();
  return slot;
}

void InstanceBufferManager::ReleaseSlot(uint32_t slot) {
  slots_[slot].entry.reset();
  slots_[slot].generation++;
  free_list_.push_back(slot);
}
