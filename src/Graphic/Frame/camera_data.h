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

inline CameraData MakeScreenSpaceCamera(float width, float height) {
  CameraData camera;
  DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
  DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, width, height, 0.0f, -10.0f, 100.0f);
  StoreMatrixToCameraData(camera, view, proj);
  camera.position = {0, 0, 0};
  camera.forward = {0, 0, 1};
  camera.up = {0, 1, 0};
  return camera;
}
