#pragma once
#include <DirectXMath.h>

#include <cmath>

namespace Billboard {

/**
 * @brief Create a cylindrical billboard matrix (Y-axis rotation only)
 * @param objectPos Object world position
 * @param cameraPos Camera world position
 * @param worldUp World up vector (fixed as Y-axis)
 * @return Billboard rotation matrix (rotation only, no translation/scale)
 */
inline DirectX::XMMATRIX CreateCylindricalBillboardMatrix(
  const DirectX::XMFLOAT3& objectPos, const DirectX::XMFLOAT3& cameraPos, const DirectX::XMFLOAT3& worldUp = {0.0f, 1.0f, 0.0f}) {
  using namespace DirectX;

  XMVECTOR objPos = XMLoadFloat3(&objectPos);
  XMVECTOR camPos = XMLoadFloat3(&cameraPos);
  XMVECTOR up = XMLoadFloat3(&worldUp);

  // Project camera-to-object vector onto XZ plane (eliminate Y component)
  XMVECTOR toCamera = XMVectorSubtract(camPos, objPos);
  toCamera = XMVectorSetY(toCamera, 0.0f);

  // Handle degenerate case (camera directly above/below)
  float lengthSq = XMVectorGetX(XMVector3LengthSq(toCamera));
  if (lengthSq < 0.0001f) {
    return XMMatrixIdentity();
  }

  // Calculate forward and right vectors on XZ plane
  XMVECTOR forward = XMVector3Normalize(XMVectorNegate(toCamera));
  XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));

  // Construct rotation matrix (right, up, forward as basis vectors)
  XMMATRIX billboardRot = XMMatrixIdentity();
  billboardRot.r[0] = XMVectorSetW(right, 0.0f);
  billboardRot.r[1] = XMVectorSetW(up, 0.0f);
  billboardRot.r[2] = XMVectorSetW(forward, 0.0f);

  return billboardRot;
}

/**
 * @brief Create a spherical billboard matrix (full rotation facing camera)
 * @param objectPos Object world position
 * @param cameraPos Camera world position
 * @param cameraUp Camera up vector
 * @return Billboard rotation matrix (rotation only, no translation/scale)
 *
 * @note filling camera up can let sprite follow camera's yaw
 */
inline DirectX::XMMATRIX CreateSphericalBillboardMatrix(
  const DirectX::XMFLOAT3& objectPos, const DirectX::XMFLOAT3& cameraPos, const DirectX::XMFLOAT3& cameraUp = {0.0f, 1.0f, 0.0f}) {
  using namespace DirectX;

  XMVECTOR objPos = XMLoadFloat3(&objectPos);
  XMVECTOR camPos = XMLoadFloat3(&cameraPos);
  XMVECTOR camUp = XMLoadFloat3(&cameraUp);

  // Calculate forward (object to camera direction)
  XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(camPos, objPos));

  // Check for gimbal lock case (forward parallel to up)
  float dotProduct = std::abs(XMVectorGetX(XMVector3Dot(forward, camUp)));
  if (dotProduct > 0.999f) {
    // Use alternative up vector (world forward) when parallel
    camUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
  }

  // Calculate right (perpendicular to forward and up)
  XMVECTOR right = XMVector3Normalize(XMVector3Cross(camUp, forward));

  // Recalculate up to ensure orthogonality
  XMVECTOR up = XMVector3Cross(forward, right);

  // Construct rotation matrix (right, up, forward as basis vectors)
  XMMATRIX billboardRot = XMMatrixIdentity();
  billboardRot.r[0] = XMVectorSetW(right, 0.0f);
  billboardRot.r[1] = XMVectorSetW(up, 0.0f);
  billboardRot.r[2] = XMVectorSetW(forward, 0.0f);

  return billboardRot;
}

}  // namespace Billboard
