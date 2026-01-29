#pragma once
#include <DirectXMath.h>

struct CameraData {
  DirectX::XMFLOAT4X4 view;
  DirectX::XMFLOAT4X4 proj;
  DirectX::XMFLOAT4X4 view_proj;
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT3 forward;
  DirectX::XMFLOAT3 up;
};

inline void StoreMatrixToCameraData(CameraData& data, DirectX::XMMATRIX view, DirectX::XMMATRIX proj) {
  DirectX::XMStoreFloat4x4(&data.view, view);
  DirectX::XMStoreFloat4x4(&data.proj, proj);
  DirectX::XMStoreFloat4x4(&data.view_proj, view * proj);
}
