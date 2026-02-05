#pragma once
#include <DirectXMath.h>

#include <cmath>

#include "Framework/Math/Math.h"

using Math::Matrix4;
using Math::Vector3;

namespace Billboard {

inline Matrix4 CreateCylindricalBillboardMatrix(const Vector3& objectPos, const Vector3& cameraPos, const Vector3& worldUp = Vector3::Up) {
  using namespace DirectX;

  XMVECTOR objPos = XMVECTOR(objectPos);
  XMVECTOR camPos = XMVECTOR(cameraPos);
  XMVECTOR up = XMVECTOR(worldUp);

  XMVECTOR toCamera = XMVectorSubtract(camPos, objPos);
  toCamera = XMVectorSetY(toCamera, 0.0f);

  float lengthSq = XMVectorGetX(XMVector3LengthSq(toCamera));
  if (lengthSq < 0.0001f) {
    return Matrix4::Identity;
  }

  XMVECTOR forward = XMVector3Normalize(XMVectorNegate(toCamera));
  XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));

  XMMATRIX billboardRot = XMMatrixIdentity();
  billboardRot.r[0] = XMVectorSetW(right, 0.0f);
  billboardRot.r[1] = XMVectorSetW(up, 0.0f);
  billboardRot.r[2] = XMVectorSetW(forward, 0.0f);

  return Matrix4(billboardRot);
}

inline Matrix4 CreateSphericalBillboardMatrix(const Vector3& objectPos, const Vector3& cameraPos, const Vector3& cameraUp = Vector3::Up) {
  using namespace DirectX;

  XMVECTOR objPos = XMVECTOR(objectPos);
  XMVECTOR camPos = XMVECTOR(cameraPos);
  XMVECTOR camUp = XMVECTOR(cameraUp);

  XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(camPos, objPos));

  float dotProduct = std::abs(XMVectorGetX(XMVector3Dot(forward, camUp)));
  if (dotProduct > 0.999f) {
    camUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
  }

  XMVECTOR right = XMVector3Normalize(XMVector3Cross(camUp, forward));
  XMVECTOR up = XMVector3Cross(forward, right);

  XMMATRIX billboardRot = XMMatrixIdentity();
  billboardRot.r[0] = XMVectorSetW(right, 0.0f);
  billboardRot.r[1] = XMVectorSetW(up, 0.0f);
  billboardRot.r[2] = XMVectorSetW(forward, 0.0f);

  return Matrix4(billboardRot);
}

}  // namespace Billboard
