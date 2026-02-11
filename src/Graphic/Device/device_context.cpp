#include "device_context.h"

#include <string>
#include <vector>

#include "Framework/Logging/logger.h"

namespace gfx {

std::unique_ptr<DeviceContext> DeviceContext::Create(const CreateInfo& info) {
  auto context = std::unique_ptr<DeviceContext>(new DeviceContext());
  if (!context->Initialize(info)) {
    return nullptr;
  }
  return context;
}

bool DeviceContext::Initialize(const CreateInfo& info) {
  if (!CreateFactory(info.enable_debug_layer)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create DXGI factory");
    return false;
  }

  if (!CreateDevice(info.require_rtx)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create D3D12 device");
    return false;
  }

  if (!QueryFeatureSupport()) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to query feature support");
    return false;
  }

  return true;
}

bool DeviceContext::EnableDebugLayer() {
  ComPtr<ID3D12Debug> debug_layer;
  if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_layer)))) {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Graphic, Logger::Here(), "Failed to get debug interface");
    return false;
  }

  debug_layer->EnableDebugLayer();
  Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "D3D12 debug layer enabled");
  return true;
}

bool DeviceContext::CreateFactory(bool enable_debug) {
  UINT dxgi_factory_flags = 0;

  if (enable_debug) {
    EnableDebugLayer();
    dxgi_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
  }

  HRESULT hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory_));
  if (FAILED(hr)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "CreateDXGIFactory2 failed: 0x{:08X}", hr);
    return false;
  }

  return true;
}

bool DeviceContext::CreateDevice([[maybe_unused]] bool require_rtx) {
  std::vector<ComPtr<IDXGIAdapter>> adapters;
  ComPtr<IDXGIAdapter> selected_adapter;

  for (UINT i = 0; factory_->EnumAdapters(i, &selected_adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
    adapters.push_back(selected_adapter);
    selected_adapter.Reset();
  }

  // Prefer NVIDIA adapter
  for (const auto& adapter : adapters) {
    DXGI_ADAPTER_DESC desc{};
    adapter->GetDesc(&desc);

    std::wstring desc_str = desc.Description;
    if (desc_str.find(L"NVIDIA") != std::wstring::npos) {
      selected_adapter = adapter;
      Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "Selected NVIDIA adapter");
      break;
    }
  }

  if (!selected_adapter && !adapters.empty()) {
    selected_adapter = adapters[0];
    Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "Using first available adapter");
  }

  constexpr D3D_FEATURE_LEVEL feature_levels[] = {
    D3D_FEATURE_LEVEL_12_2,
    D3D_FEATURE_LEVEL_12_1,
    D3D_FEATURE_LEVEL_12_0,
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
  };

  for (D3D_FEATURE_LEVEL level : feature_levels) {
    HRESULT hr = D3D12CreateDevice(selected_adapter.Get(), level, IID_PPV_ARGS(&device_));
    if (SUCCEEDED(hr) && device_) {
      feature_level_ = level;
      Logger::LogFormat(LogLevel::Info,
        LogCategory::Graphic,
        Logger::Here(),
        "Created D3D12 device with feature level 0x{:X}",
        static_cast<uint32_t>(level));
      return true;
    }
  }

  Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create D3D12 device at any feature level");
  return false;
}

bool DeviceContext::QueryFeatureSupport() {
  // Check bindless sampler support (Resource Binding Tier 3)
  D3D12_FEATURE_DATA_D3D12_OPTIONS options{};
  if (SUCCEEDED(device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)))) {
    supports_bindless_samplers_ = (options.ResourceBindingTier >= D3D12_RESOURCE_BINDING_TIER_3);
    Logger::LogFormat(LogLevel::Info,
      LogCategory::Graphic,
      Logger::Here(),
      "Resource Binding Tier: {}, Bindless samplers: {}",
      static_cast<int>(options.ResourceBindingTier),
      supports_bindless_samplers_ ? "supported" : "not supported");
  } else {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Graphic, Logger::Here(), "Failed to query D3D12_OPTIONS");
    supports_bindless_samplers_ = false;
  }

  // Check raytracing support (DXR 1.0)
  D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5{};
  if (SUCCEEDED(device_->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)))) {
    supports_raytracing_ = (options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0);
    Logger::LogFormat(LogLevel::Info,
      LogCategory::Graphic,
      Logger::Here(),
      "Raytracing Tier: {}, DXR: {}",
      static_cast<int>(options5.RaytracingTier),
      supports_raytracing_ ? "supported" : "not supported");
  } else {
    supports_raytracing_ = false;
  }

  // Check tearing support for unlocked framerate
  BOOL allow_tearing = FALSE;
  if (SUCCEEDED(factory_->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing)))) {
    supports_tearing_ = (allow_tearing == TRUE);
    Logger::LogFormat(
      LogLevel::Info, LogCategory::Graphic, Logger::Here(), "Tearing: {}", supports_tearing_ ? "supported" : "not supported");
  } else {
    supports_tearing_ = false;
  }

  return true;
}

}  // namespace gfx
