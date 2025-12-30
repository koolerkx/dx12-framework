#pragma once
#include <DirectXMath.h>

struct CameraData {
  DirectX::XMFLOAT4X4 view;
  DirectX::XMFLOAT4X4 proj;
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT3 forward;
  DirectX::XMFLOAT3 up;

  DirectX::XMMATRIX GetViewProjMatrix() const {
    using namespace DirectX;
    XMMATRIX v = XMLoadFloat4x4(&view);
    XMMATRIX p = XMLoadFloat4x4(&proj);
    return v * p;
  }
};
