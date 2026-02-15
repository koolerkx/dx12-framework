#include "descriptor_heap_manager.h"

#include "Framework/Logging/logger.h"

bool DescriptorHeapManager::Initialize(ID3D12Device* device, uint32_t frame_count, const DescriptorHeapConfig& config) {
  config_ = config;

  // Independent Heaps
  if (!rtv_heap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, config_.rtv_capacity, false)) return false;
  if (!dsv_heap_.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, config_.dsv_capacity, false)) return false;

  // Global SRV Heap (The Big One)
  const uint32_t static_size = config_.srv_static_size;
  const uint32_t frame_dynamic_size = config_.srv_frame_dynamic_size;
  const uint32_t total_srv_size = static_size + (frame_dynamic_size * frame_count);

  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  desc.NumDescriptors = total_srv_size;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&global_srv_heap_)))) return false;
  global_srv_heap_->SetName(L"Global_SRV_Heap");
  rtv_heap_.GetHeap()->SetName(L"RTV_Heap");
  dsv_heap_.GetHeap()->SetName(L"DSV_Heap");

  uint32_t increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  // Initialize Allocators
  srv_static_.InitializeSubAllocation(global_srv_heap_.Get(), 0, static_size, increment);

  srv_dynamic_.resize(frame_count);
  for (uint32_t i = 0; i < frame_count; ++i) {
    srv_dynamic_[i] = std::make_unique<DescriptorHeapAllocator>();
    uint32_t offset = static_size + (i * frame_dynamic_size);
    srv_dynamic_[i]->InitializeSubAllocation(global_srv_heap_.Get(), offset, frame_dynamic_size, increment);
  }

  // Create Sampler Heap
  if (!InitializeSamplerHeap(device)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Resource, Logger::Here(), "DescriptorHeapManager: Failed to initialize sampler heap");
    return false;
  }

  return true;
}

void DescriptorHeapManager::BeginFrame(uint32_t frame_index) {
  // Only reset the dynamic region for this frame
  srv_dynamic_[frame_index]->Reset();
}

void DescriptorHeapManager::SetDescriptorHeaps(ID3D12GraphicsCommandList* cmdList) {
  ID3D12DescriptorHeap* heaps[] = {global_srv_heap_.Get(), sampler_heap_.Get()};
  cmdList->SetDescriptorHeaps(2, heaps);
}

bool DescriptorHeapManager::InitializeSamplerHeap(ID3D12Device* device) {
  // Create shader-visible sampler heap
  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
  desc.NumDescriptors = config_.sampler_capacity;
  desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&sampler_heap_)))) {
    return false;
  }
  sampler_heap_->SetName(L"Bindless_Sampler_Heap");

  // Create 5 default samplers
  CreateDefaultSamplers(device);

  return true;
}

void DescriptorHeapManager::CreateDefaultSamplers(ID3D12Device* device) {
  struct SamplerPreset {
    D3D12_FILTER filter;
    D3D12_TEXTURE_ADDRESS_MODE address_mode;
    UINT max_anisotropy;
  };

  constexpr SamplerPreset SAMPLER_PRESETS[] = {
    {D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_WRAP,  0},
    {D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,  0},
    {D3D12_FILTER_ANISOTROPIC,        D3D12_TEXTURE_ADDRESS_MODE_WRAP,  16},
    {D3D12_FILTER_MIN_MAG_MIP_POINT,  D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0},
    {D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0},
  };

  D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = sampler_heap_->GetCPUDescriptorHandleForHeapStart();
  uint32_t increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

  for (const auto& preset : SAMPLER_PRESETS) {
    D3D12_SAMPLER_DESC desc = {};
    desc.Filter = preset.filter;
    desc.AddressU = preset.address_mode;
    desc.AddressV = preset.address_mode;
    desc.AddressW = preset.address_mode;
    desc.MaxAnisotropy = preset.max_anisotropy;
    desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    desc.MinLOD = 0.0f;
    desc.MaxLOD = D3D12_FLOAT32_MAX;
    device->CreateSampler(&desc, cpu_handle);
    cpu_handle.ptr += increment;
  }
}
