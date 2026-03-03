#include "Resource/Mesh/mesh_buffer_pool.h"

#include <algorithm>
#include <string>

#include "Framework/Logging/logger.h"

namespace {

ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device, uint64_t size, const std::wstring& name) {
  D3D12_HEAP_PROPERTIES heap_props = {};
  heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;

  D3D12_RESOURCE_DESC desc = {};
  desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Width = size;
  desc.Height = 1;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.SampleDesc.Count = 1;
  desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  ComPtr<ID3D12Resource> resource;
  HRESULT hr = device->CreateCommittedResource(
    &heap_props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource));

  if (FAILED(hr)) return nullptr;

  resource->SetName(name.c_str());
  return resource;
}

}  // namespace

bool MeshBufferPool::Initialize(const CreateInfo& info, const MeshBufferPoolConfig& config) {
  device_ = info.device;
  execute_sync_ = info.execute_sync;
  get_current_fence_value_ = info.get_current_fence_value;

  if (!device_ || !execute_sync_ || !get_current_fence_value_) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "MeshBufferPool: invalid CreateInfo");
    return false;
  }

  vertex_capacity_ = config.initial_vertex_capacity;
  index_capacity_ = config.initial_index_capacity;
  max_mesh_count_ = config.max_mesh_count;

  vertex_buffer_ = CreateDefaultBuffer(device_, static_cast<uint64_t>(vertex_capacity_) * sizeof(ModelVertex), L"MeshBufferPool_Vertices");
  if (!vertex_buffer_) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "MeshBufferPool: failed to create vertex buffer");
    return false;
  }

  index_buffer_ = CreateDefaultBuffer(device_, static_cast<uint64_t>(index_capacity_) * sizeof(uint32_t), L"MeshBufferPool_Indices");
  if (!index_buffer_) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "MeshBufferPool: failed to create index buffer");
    return false;
  }

  descriptor_buffer_ =
    CreateDefaultBuffer(device_, static_cast<uint64_t>(max_mesh_count_) * sizeof(MeshDescriptor), L"MeshBufferPool_Descriptors");
  if (!descriptor_buffer_) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "MeshBufferPool: failed to create descriptor buffer");
    return false;
  }

  vertex_allocator_.Initialize(vertex_capacity_);
  index_allocator_.Initialize(index_capacity_);

  Logger::LogFormat(LogLevel::Info,
    LogCategory::Resource,
    Logger::Here(),
    "MeshBufferPool initialized: {} vertices, {} indices, {} max meshes",
    vertex_capacity_,
    index_capacity_,
    max_mesh_count_);

  return true;
}

MeshAllocation MeshBufferPool::Allocate(std::span<const ModelVertex> vertices, std::span<const uint32_t> indices) {
  if (vertices.empty() || indices.empty()) return {MeshHandle::Invalid(), false};

  auto vertex_count = static_cast<uint32_t>(vertices.size());
  auto index_count = static_cast<uint32_t>(indices.size());

  uint32_t vertex_offset = vertex_allocator_.Allocate(vertex_count);
  if (vertex_offset == FreeBlockAllocator::INVALID_OFFSET) {
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Resource,
      Logger::Here(),
      "MeshBufferPool: vertex allocation failed ({} vertices requested)",
      vertex_count);
    return {MeshHandle::Invalid(), false};
  }

  uint32_t index_offset = index_allocator_.Allocate(index_count);
  if (index_offset == FreeBlockAllocator::INVALID_OFFSET) {
    vertex_allocator_.Free(vertex_offset, vertex_count);
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Resource,
      Logger::Here(),
      "MeshBufferPool: index allocation failed ({} indices requested)",
      index_count);
    return {MeshHandle::Invalid(), false};
  }

  uint32_t slot = AllocateSlot();
  if (slot == UINT32_MAX) {
    vertex_allocator_.Free(vertex_offset, vertex_count);
    index_allocator_.Free(index_offset, index_count);
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Resource, Logger::Here(), "MeshBufferPool: no available descriptor slots (max {})", max_mesh_count_);
    return {MeshHandle::Invalid(), false};
  }

  UploadRegion(vertex_buffer_.Get(),
    static_cast<uint64_t>(vertex_offset) * sizeof(ModelVertex),
    vertices.data(),
    static_cast<uint64_t>(vertex_count) * sizeof(ModelVertex));

  UploadRegion(index_buffer_.Get(),
    static_cast<uint64_t>(index_offset) * sizeof(uint32_t),
    indices.data(),
    static_cast<uint64_t>(index_count) * sizeof(uint32_t));

  auto& slot_data = slots_[slot];
  slot_data.descriptor = {vertex_offset, vertex_count, index_offset, index_count};
  slot_data.occupied = true;

  UpdateDescriptorOnGpu(slot);

  MeshHandle handle;
  handle.index = slot;
  handle.generation = slot_data.generation;

  return {handle, true};
}

void MeshBufferPool::Free(MeshHandle handle) {
  if (!IsValid(handle)) return;

  const auto& slot_data = slots_[handle.index];
  const auto& desc = slot_data.descriptor;

  pending_frees_.push_back({
    handle.index,
    handle.generation,
    desc.vertex_offset,
    desc.vertex_count,
    desc.index_offset,
    desc.index_count,
    get_current_fence_value_(),
  });
}

void MeshBufferPool::ProcessDeferredFrees(uint64_t completed_fence_value) {
  auto it = std::remove_if(pending_frees_.begin(), pending_frees_.end(), [&](const PendingFree& pf) {
    if (pf.fence_value <= completed_fence_value) {
      if (pf.slot < slots_.size() && slots_[pf.slot].generation == pf.generation) {
        vertex_allocator_.Free(pf.vertex_offset, pf.vertex_count);
        index_allocator_.Free(pf.index_offset, pf.index_count);
        ReleaseSlot(pf.slot);
      }
      return true;
    }
    return false;
  });
  pending_frees_.erase(it, pending_frees_.end());
}

void MeshBufferPool::Shutdown() {
  for (auto& pf : pending_frees_) {
    if (pf.slot < slots_.size() && slots_[pf.slot].generation == pf.generation) {
      ReleaseSlot(pf.slot);
    }
  }
  pending_frees_.clear();

  slots_.clear();
  free_slots_.clear();

  vertex_buffer_.Reset();
  index_buffer_.Reset();
  descriptor_buffer_.Reset();

  device_ = nullptr;
}

D3D12_VERTEX_BUFFER_VIEW MeshBufferPool::GetVertexBufferView() const {
  D3D12_VERTEX_BUFFER_VIEW view = {};
  if (vertex_buffer_) {
    view.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    view.SizeInBytes = vertex_capacity_ * sizeof(ModelVertex);
    view.StrideInBytes = sizeof(ModelVertex);
  }
  return view;
}

D3D12_INDEX_BUFFER_VIEW MeshBufferPool::GetIndexBufferView() const {
  D3D12_INDEX_BUFFER_VIEW view = {};
  if (index_buffer_) {
    view.BufferLocation = index_buffer_->GetGPUVirtualAddress();
    view.SizeInBytes = index_capacity_ * sizeof(uint32_t);
    view.Format = DXGI_FORMAT_R32_UINT;
  }
  return view;
}

D3D12_GPU_VIRTUAL_ADDRESS MeshBufferPool::GetDescriptorBufferAddress() const {
  return descriptor_buffer_ ? descriptor_buffer_->GetGPUVirtualAddress() : 0;
}

const MeshDescriptor* MeshBufferPool::GetDescriptor(MeshHandle handle) const {
  if (!IsValid(handle)) return nullptr;
  return &slots_[handle.index].descriptor;
}

bool MeshBufferPool::IsValid(MeshHandle handle) const {
  if (!handle.IsValid()) return false;
  if (handle.index >= slots_.size()) return false;
  const auto& slot = slots_[handle.index];
  return slot.occupied && slot.generation == handle.generation;
}

void MeshBufferPool::UploadRegion(ID3D12Resource* dest, uint64_t dest_offset, const void* data, uint64_t size) {
  D3D12_HEAP_PROPERTIES upload_props = {};
  upload_props.Type = D3D12_HEAP_TYPE_UPLOAD;

  D3D12_RESOURCE_DESC desc = {};
  desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Width = size;
  desc.Height = 1;
  desc.DepthOrArraySize = 1;
  desc.MipLevels = 1;
  desc.Format = DXGI_FORMAT_UNKNOWN;
  desc.SampleDesc.Count = 1;
  desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  ComPtr<ID3D12Resource> upload_buffer;
  HRESULT hr = device_->CreateCommittedResource(
    &upload_props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_buffer));

  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "MeshBufferPool: failed to create upload buffer");
    return;
  }

  void* mapped = nullptr;
  D3D12_RANGE read_range = {0, 0};
  hr = upload_buffer->Map(0, &read_range, &mapped);
  if (FAILED(hr)) return;

  memcpy(mapped, data, static_cast<size_t>(size));
  upload_buffer->Unmap(0, nullptr);

  execute_sync_([&](ID3D12GraphicsCommandList* cmd) { cmd->CopyBufferRegion(dest, dest_offset, upload_buffer.Get(), 0, size); });
}

uint32_t MeshBufferPool::AllocateSlot() {
  if (!free_slots_.empty()) {
    uint32_t slot = free_slots_.back();
    free_slots_.pop_back();
    return slot;
  }

  if (static_cast<uint32_t>(slots_.size()) >= max_mesh_count_) return UINT32_MAX;

  auto slot = static_cast<uint32_t>(slots_.size());
  slots_.emplace_back();
  return slot;
}

void MeshBufferPool::ReleaseSlot(uint32_t slot) {
  slots_[slot].occupied = false;
  slots_[slot].generation++;
  free_slots_.push_back(slot);
}

void MeshBufferPool::UpdateDescriptorOnGpu(uint32_t slot) {
  const auto& desc = slots_[slot].descriptor;
  uint64_t offset = static_cast<uint64_t>(slot) * sizeof(MeshDescriptor);
  UploadRegion(descriptor_buffer_.Get(), offset, &desc, sizeof(MeshDescriptor));
}
