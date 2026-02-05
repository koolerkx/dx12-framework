#pragma once
#include <DirectXMath.h>

#include "Framework/Math/Math.h"

using Math::Matrix4;
using Math::Vector3;

struct CameraData {
  Matrix4 view;
  Matrix4 proj;
  Matrix4 view_proj;
  Vector3 position;
  Vector3 forward;
  Vector3 up;
};

inline void StoreMatrixToCameraData(CameraData& data, DirectX::XMMATRIX view, DirectX::XMMATRIX proj) {
  data.view = Matrix4(view);
  data.proj = Matrix4(proj);
  data.view_proj = Matrix4(view * proj);
}

inline CameraData MakeScreenSpaceCamera(float width, float height) {
  CameraData camera;
  DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
  DirectX::XMMATRIX proj = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, width, height, 0.0f, -10.0f, 100.0f);
  StoreMatrixToCameraData(camera, view, proj);
  camera.position = Vector3::Zero;
  camera.forward = Vector3::Forward;
  camera.up = Vector3::Up;
  return camera;
}
