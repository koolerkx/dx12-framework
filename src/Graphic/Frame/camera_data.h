#pragma once
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

inline void StoreMatrixToCameraData(CameraData& data, const Matrix4& view, const Matrix4& proj) {
  data.view = view;
  data.proj = proj;
  data.view_proj = view * proj;
}

inline CameraData MakeScreenSpaceCamera(float width, float height) {
  CameraData camera;
  Matrix4 proj = Matrix4::CreateOrthographicOffCenter(0.0f, width, height, 0.0f, -10.0f, 100.0f);
  StoreMatrixToCameraData(camera, Matrix4::Identity, proj);
  camera.position = Vector3::Zero;
  camera.forward = Vector3::Forward;
  camera.up = Vector3::Up;
  return camera;
}
