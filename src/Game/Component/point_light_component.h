#pragma once

#include "Component/renderer_component.h"
#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Framework/Math/Math.h"
#include "Framework/Serialize/serialize_node.h"
#include "Framework/Render/frame_packet.h"
#include "game_object.h"

class PointLightComponent : public RendererComponent<PointLightComponent> {
 public:
  struct Props {
    Math::Vector3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float radius = 10.0f;
    float falloff = 2.0f;
  };

  using RendererComponent::RendererComponent;

  PointLightComponent(GameObject* owner, const Props& props) : RendererComponent(owner) {
    color_ = props.color;
    intensity_ = props.intensity;
    radius_ = props.radius;
    falloff_ = props.falloff;
  }

  void SetColor(const Math::Vector3& color) {
    color_ = color;
  }
  void SetIntensity(float intensity) {
    intensity_ = intensity;
  }
  void SetRadius(float radius) {
    radius_ = radius;
  }
  void SetFalloff(float falloff) {
    falloff_ = falloff;
  }

  Math::Vector3 GetColor() const {
    return color_;
  }
  float GetIntensity() const {
    return intensity_;
  }
  float GetRadius() const {
    return radius_;
  }
  float GetFalloff() const {
    return falloff_;
  }

  void OnSerialize(framework::SerializeNode& node) const override {
    node.WriteVec3("Color", color_.x, color_.y, color_.z);
    node.Write("Intensity", intensity_);
    node.Write("Radius", radius_);
    node.Write("Falloff", falloff_);
  }

  void OnDeserialize(const framework::SerializeNode& node) override {
    node.ReadVec3("Color", color_.x, color_.y, color_.z);
    intensity_ = node.ReadFloat("Intensity", intensity_);
    radius_ = node.ReadFloat("Radius", radius_);
    falloff_ = node.ReadFloat("Falloff", falloff_);
  }

  struct EditorData {
    Math::Vector3 color;
    float intensity;
    float radius;
    float falloff;
  };

  EditorData GetEditorData() const {
    return {color_, intensity_, radius_, falloff_};
  }

  void ApplyEditorData(const EditorData& data) {
    color_ = data.color;
    intensity_ = data.intensity;
    radius_ = data.radius;
    falloff_ = data.falloff;
  }

  void OnDebugDraw(DebugDrawer& drawer) override {
    auto position = GetOwner()->GetTransform()->GetWorldPosition();
    drawer.DrawWireSphere(position, radius_, Math::Vector4(color_.x, color_.y, color_.z, 1.0f));
  }

  void OnRender(FramePacket& packet) override {
    auto* transform = GetOwner()->GetTransform();
    if (!transform) return;

    PointLightEntry entry;
    entry.position = transform->GetWorldPosition();
    entry.intensity = intensity_;
    entry.color = color_;
    entry.radius = radius_;
    entry.falloff = falloff_;
    packet.AddPointLight(std::move(entry));
  }

 private:
  Math::Vector3 color_ = {1.0f, 1.0f, 1.0f};
  float intensity_ = 1.0f;
  float radius_ = 10.0f;
  float falloff_ = 2.0f;
};
