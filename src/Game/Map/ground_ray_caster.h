#pragma once

#include <optional>

#include "Framework/Math/Math.h"
#include "Framework/Render/camera_data.h"

namespace GroundRayCaster {

inline Math::Ray ScreenToWorldRay(float mouse_x, float mouse_y, float screen_w, float screen_h, const CameraData& camera) {
  float ndc_x = (2.0f * mouse_x / screen_w) - 1.0f;
  float ndc_y = 1.0f - (2.0f * mouse_y / screen_h);

  Math::Vector4 clip_near(ndc_x, ndc_y, 0.0f, 1.0f);
  Math::Vector4 clip_far(ndc_x, ndc_y, 1.0f, 1.0f);

  auto unproject = [](const Math::Vector4& clip, const Math::Matrix4& inv_proj, const Math::Matrix4& inv_view) -> Math::Vector3 {
    DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&clip);
    DirectX::XMMATRIX ip = DirectX::XMLoadFloat4x4(&inv_proj);
    DirectX::XMMATRIX iv = DirectX::XMLoadFloat4x4(&inv_view);

    v = DirectX::XMVector4Transform(v, ip);
    v = DirectX::XMVectorDivide(v, DirectX::XMVectorSplatW(v));
    v = DirectX::XMVector4Transform(v, iv);
    v = DirectX::XMVectorDivide(v, DirectX::XMVectorSplatW(v));

    Math::Vector3 result;
    DirectX::XMStoreFloat3(&result, v);
    return result;
  };

  Math::Vector3 world_near = unproject(clip_near, camera.inv_proj, camera.inv_view);
  Math::Vector3 world_far = unproject(clip_far, camera.inv_proj, camera.inv_view);
  Math::Vector3 direction = (world_far - world_near).Normalized();

  return Math::Ray(world_near, direction);
}

inline std::optional<Math::Vector2> ScreenToGroundXZ(
  float mouse_x, float mouse_y, float screen_w, float screen_h, const CameraData& camera, float ground_y = 0.0f) {
  Math::Ray ray = ScreenToWorldRay(mouse_x, mouse_y, screen_w, screen_h, camera);
  Math::Plane ground(Math::Vector3::UnitY, -ground_y);

  float distance = 0.0f;
  if (!Math::Intersects(ray, ground, distance)) return std::nullopt;

  Math::Vector3 hit = ray.PointAt(distance);
  return Math::Vector2(hit.x, hit.z);
}

}  // namespace GroundRayCaster
