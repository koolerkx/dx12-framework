#pragma once
#include "Framework/Math/Math.h"

using Math::Matrix4;
using Math::Vector3;

namespace Billboard {

inline Matrix4 CreateCylindricalBillboardMatrix(
    const Vector3& objectPos, const Vector3& cameraPos,
    const Vector3& worldUp = Vector3::Up) {
  Vector3 toCamera = cameraPos - objectPos;
  toCamera.y = 0.0f;
  if (toCamera.LengthSquared() < 0.0001f) return Matrix4::Identity;
  return Matrix4::FaceTo(objectPos, objectPos + toCamera, worldUp);
}

inline Matrix4 CreateSphericalBillboardMatrix(
    const Vector3& objectPos, const Vector3& cameraPos,
    const Vector3& cameraUp = Vector3::Up) {
  return Matrix4::FaceTo(objectPos, cameraPos, cameraUp);
}

}  // namespace Billboard
