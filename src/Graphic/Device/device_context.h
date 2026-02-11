#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <memory>

#include "Core/types.h"

namespace gfx {

class DeviceContext {
 public:
  struct CreateInfo {
    bool enable_debug_layer = false;
    bool require_rtx = false;
  };

  [[nodiscard]] static std::unique_ptr<DeviceContext> Create(const CreateInfo& info);

  DeviceContext(const DeviceContext&) = delete;
  DeviceContext& operator=(const DeviceContext&) = delete;
  ~DeviceContext() = default;

  ID3D12Device5* GetDevice() const {
    return device_.Get();
  }

  IDXGIFactory6* GetFactory() const {
    return factory_.Get();
  }

  bool SupportsBindlessSamplers() const {
    return supports_bindless_samplers_;
  }

  bool SupportsRaytracing() const {
    return supports_raytracing_;
  }

  bool SupportsTearing() const {
    return supports_tearing_;
  }

  D3D_FEATURE_LEVEL GetFeatureLevel() const {
    return feature_level_;
  }

 private:
  DeviceContext() = default;
  bool Initialize(const CreateInfo& info);

  bool EnableDebugLayer();
  bool CreateFactory(bool enable_debug);
  bool CreateDevice(bool require_rtx);
  bool QueryFeatureSupport();

  ComPtr<ID3D12Device5> device_;
  ComPtr<IDXGIFactory6> factory_;

  bool supports_bindless_samplers_ = false;
  bool supports_raytracing_ = false;
  bool supports_tearing_ = false;
  D3D_FEATURE_LEVEL feature_level_ = D3D_FEATURE_LEVEL_11_0;
};

}  // namespace gfx
