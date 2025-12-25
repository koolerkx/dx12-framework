#include "descriptor_heap_manager.h"

#include <cassert>
#include <iostream>
#include <mutex>

bool DescriptorHeapManager::Initalize(ID3D12Device* device) {
  assert(device != nullptr);

  if (!rtv_heap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, config_.rtv_capacity, false)) {
    std::cerr << "Failed to initialize RTV heap" << std::endl;
    return false;
  }
  if (!dsv_heap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, config_.dsv_capacity, false)) {
    std::cerr << "Failed to initialize DSV heap" << std::endl;
    return false;
  }
  if (!srv_heap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, config_.srv_capacity, true)) {
    std::cerr << "Failed to initialize SRV heap" << std::endl;
    return false;
  }
  if (!sampler_heap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, config_.sampler_capacity, true)) {
    std::cerr << "Failed to initialize sampler heap" << std::endl;
    return false;
  }

  return true;
}

void DescriptorHeapManager::BeginFrame() {
  std::lock_guard<std::mutex> lock(begin_frame_mutex_);
  srv_heap_.Reset();
  sampler_heap_.Reset();
}

void DescriptorHeapManager::SetDescriptorHeaps(ID3D12GraphicsCommandList* command_list) {
  assert(command_list != nullptr);

  ID3D12DescriptorHeap* heaps[] = {srv_heap_.GetHeap(), sampler_heap_.GetHeap()};

  command_list->SetDescriptorHeaps(2, heaps);
}

void DescriptorHeapManager::PrintStats() const {
  std::cout << "\n=== Descriptor Heap Statistics ===" << std::endl;
  std::cout << "RTV Heap: " << rtv_heap_.GetAllocated() << "/" << rtv_heap_.GetCapacity() << std::endl;
  std::cout << "DSV Heap: " << dsv_heap_.GetAllocated() << "/" << dsv_heap_.GetCapacity() << std::endl;
  std::cout << "SRV Heap (per-frame): " << srv_heap_.GetAllocated() << "/" << srv_heap_.GetCapacity() << std::endl;
  std::cout << "Sampler Heap (per-frame): " << sampler_heap_.GetAllocated() << "/" << sampler_heap_.GetCapacity() << std::endl;
  std::cout << "==================================\n" << std::endl;
}
