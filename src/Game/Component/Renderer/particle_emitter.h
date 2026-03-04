#pragma once

#include <functional>
#include <random>
#include <vector>

#include "Asset/asset_handle.h"
#include "Component/renderer_component.h"
#include "Component/render_settings.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Framework/Serialize/serialize_node.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Texture/texture.h"
#include "game_context.h"
#include "game_object.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector3;
using Math::Vector4;

enum class SpawnShape : uint8_t { Point, Disk, Custom };

class ParticleEmitter : public RendererComponent<ParticleEmitter> {
 public:
  struct SpawnParams {
    Vector3 offset;
    Vector3 direction;
  };

  using SpawnFn = std::function<SpawnParams(std::mt19937& rng)>;

  struct Particle {
    Vector3 position;
    Vector3 velocity;
    Vector4 color;
    float size;
    float start_size;
    float lifetime;
    float age;

    bool IsAlive() const {
      return age < lifetime;
    }
    float GetLifetimeRatio() const {
      return age / lifetime;
    }
  };

  struct Props {
    std::string texture_path;
    size_t max_particles = 100;
    float emit_rate = 10.0f;
    float particle_lifetime = 2.0f;
    Vector2 particle_size = {1.0f, 1.0f};
    Vector4 start_color = {1, 1, 1, 1};
    Vector4 end_color = {1, 1, 1, 0};
    float start_speed = 5.0f;
    float speed_variation = 2.0f;
    Vector3 gravity = {0, -9.8f, 0};
    bool loop = true;
    uint32_t burst_count = 0;
    float drag = 0.0f;
    float end_size = 1.0f;
    float lifetime_variation = 0.0f;
    float size_variation = 0.0f;
    Rendering::BlendMode blend_mode = Rendering::BlendMode::Additive;
    Vector3 spawn_offset = {0, 0, 0};
    SpawnShape spawn_shape = SpawnShape::Point;
    float spawn_radius = 1.0f;
    float fade_in_ratio = 0.0f;
    float fade_out_ratio = 0.0f;
    float emissive_intensity = 1.0f;
    float soft_distance = 0.5f;
    SpawnFn spawn_fn = SpawnFromCenter();
  };

  struct EditorData {
    float emit_rate;
    float particle_lifetime;
    Vector2 particle_size;
    Vector4 start_color;
    Vector4 end_color;
    float start_speed;
    float speed_variation;
    Vector3 gravity;
    Vector3 spawn_offset;
    float spawn_radius;
    float fade_in_ratio;
    float fade_out_ratio;
    float emissive_intensity;
    float soft_distance;
    bool loop;
  };

  ParticleEmitter(GameObject* owner);
  ParticleEmitter(GameObject* owner, const Props& props);

  void SetTexturePath(const std::string& path) {
    texture_path_ = path;
    if (path.empty()) {
      texture_ = nullptr;
      texture_handle_ = {};
      return;
    }
    auto* context = GetOwner()->GetContext();
    if (!context) return;
    texture_handle_ = context->GetAssetManager().LoadTexture(path);
    texture_ = texture_handle_.Get();
  }

  EditorData GetEditorData() const {
    return {
      emit_rate_,
      particle_lifetime_,
      particle_size_,
      start_color_,
      end_color_,
      start_speed_,
      speed_variation_,
      gravity_,
      spawn_offset_,
      spawn_radius_,
      fade_in_ratio_,
      fade_out_ratio_,
      emissive_intensity_,
      soft_distance_,
      loop_,
    };
  }

  void ApplyEditorData(const EditorData& data) {
    emit_rate_ = data.emit_rate;
    particle_lifetime_ = data.particle_lifetime;
    particle_size_ = data.particle_size;
    start_color_ = data.start_color;
    end_color_ = data.end_color;
    start_speed_ = data.start_speed;
    speed_variation_ = data.speed_variation;
    gravity_ = data.gravity;
    spawn_offset_ = data.spawn_offset;
    spawn_radius_ = data.spawn_radius;
    fade_in_ratio_ = data.fade_in_ratio;
    fade_out_ratio_ = data.fade_out_ratio;
    emissive_intensity_ = data.emissive_intensity;
    soft_distance_ = data.soft_distance;
    loop_ = data.loop;
  }

  void OnSerialize(framework::SerializeNode& node) const override {
    if (!texture_path_.empty()) {
      node.Write("Texture", texture_path_);
    }

    node.Write("MaxParticles", static_cast<uint32_t>(max_particles_));
    node.Write("EmitRate", emit_rate_);
    node.Write("ParticleLifetime", particle_lifetime_);

    node.WriteVec2("ParticleSize", particle_size_.x, particle_size_.y);
    node.WriteVec4("StartColor", start_color_.x, start_color_.y, start_color_.z, start_color_.w);
    node.WriteVec4("EndColor", end_color_.x, end_color_.y, end_color_.z, end_color_.w);
    node.Write("StartSpeed", start_speed_);
    node.Write("SpeedVariation", speed_variation_);

    node.WriteVec3("Gravity", gravity_.x, gravity_.y, gravity_.z);
    node.Write("Loop", loop_);
    node.Write("BlendMode", static_cast<int>(render_settings_.blend_mode));
    node.WriteVec3("SpawnOffset", spawn_offset_.x, spawn_offset_.y, spawn_offset_.z);
    node.Write("SpawnShape", static_cast<int>(spawn_shape_));
    node.Write("SpawnRadius", spawn_radius_);
    node.Write("FadeInRatio", fade_in_ratio_);
    node.Write("FadeOutRatio", fade_out_ratio_);
    node.Write("EmissiveIntensity", emissive_intensity_);
    node.Write("SoftDistance", soft_distance_);
    node.Write("BurstCount", burst_count_);
    node.Write("Drag", drag_);
    node.Write("EndSize", end_size_);
    node.Write("LifetimeVariation", lifetime_variation_);
    node.Write("SizeVariation", size_variation_);
  }

  void OnDeserialize(const framework::SerializeNode& node) override {
    auto tex_path = node.ReadString("Texture");
    if (!tex_path.empty()) {
      SetTexturePath(tex_path);
    }

    max_particles_ = node.ReadUint("MaxParticles", static_cast<uint32_t>(max_particles_));
    emit_rate_ = node.ReadFloat("EmitRate", emit_rate_);
    particle_lifetime_ = node.ReadFloat("ParticleLifetime", particle_lifetime_);

    node.ReadVec2("ParticleSize", particle_size_.x, particle_size_.y);
    node.ReadVec4("StartColor", start_color_.x, start_color_.y, start_color_.z, start_color_.w);
    node.ReadVec4("EndColor", end_color_.x, end_color_.y, end_color_.z, end_color_.w);
    start_speed_ = node.ReadFloat("StartSpeed", start_speed_);
    speed_variation_ = node.ReadFloat("SpeedVariation", speed_variation_);

    node.ReadVec3("Gravity", gravity_.x, gravity_.y, gravity_.z);
    loop_ = node.ReadBool("Loop", loop_);
    render_settings_.blend_mode =
      static_cast<Rendering::BlendMode>(node.ReadInt("BlendMode", static_cast<int>(render_settings_.blend_mode)));
    node.ReadVec3("SpawnOffset", spawn_offset_.x, spawn_offset_.y, spawn_offset_.z);
    spawn_shape_ = static_cast<SpawnShape>(node.ReadInt("SpawnShape", static_cast<int>(spawn_shape_)));
    switch (spawn_shape_) {
      case SpawnShape::Disk: spawn_fn_ = SpawnFromDisk(); break;
      default: spawn_fn_ = SpawnFromCenter(); break;
    }
    spawn_radius_ = node.ReadFloat("SpawnRadius", spawn_radius_);
    fade_in_ratio_ = node.ReadFloat("FadeInRatio", fade_in_ratio_);
    fade_out_ratio_ = node.ReadFloat("FadeOutRatio", fade_out_ratio_);
    emissive_intensity_ = node.ReadFloat("EmissiveIntensity", emissive_intensity_);
    soft_distance_ = node.ReadFloat("SoftDistance", soft_distance_);
    burst_count_ = node.ReadUint("BurstCount", burst_count_);
    drag_ = node.ReadFloat("Drag", drag_);
    end_size_ = node.ReadFloat("EndSize", end_size_);
    lifetime_variation_ = node.ReadFloat("LifetimeVariation", lifetime_variation_);
    size_variation_ = node.ReadFloat("SizeVariation", size_variation_);
  }

  void Play();
  void Stop();
  void Clear();

  void SetOnAllDeadCallback(std::function<void()> callback) {
    on_all_dead_ = std::move(callback);
  }

  bool IsPlaying() const {
    return is_playing_;
  }

  void OnUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;

  static SpawnFn SpawnFromCenter();
  static SpawnFn SpawnFromDisk();

 private:
  void EmitParticles(float dt);
  void UpdateParticles(float dt);
  void RemoveDeadParticles();

  std::vector<Particle> particles_;

  Texture* texture_ = nullptr;
  std::string texture_path_;
  AssetHandle<Texture> texture_handle_;
  size_t max_particles_ = 100;
  float emit_rate_ = 10.0f;
  float particle_lifetime_ = 2.0f;
  Vector2 particle_size_ = {1.0f, 1.0f};
  Vector4 start_color_ = {1, 1, 1, 1};
  Vector4 end_color_ = {1, 1, 1, 0};
  float start_speed_ = 5.0f;
  float speed_variation_ = 2.0f;
  Vector3 gravity_ = {0, -9.8f, 0};
  bool loop_ = true;

  Vector3 spawn_offset_ = {0, 0, 0};
  SpawnShape spawn_shape_ = SpawnShape::Point;
  float spawn_radius_ = 1.0f;
  float fade_in_ratio_ = 0.0f;
  float fade_out_ratio_ = 0.0f;
  float emissive_intensity_ = 1.0f;
  float soft_distance_ = 0.5f;

  uint32_t burst_count_ = 0;
  float drag_ = 0.0f;
  float end_size_ = 1.0f;
  float lifetime_variation_ = 0.0f;
  float size_variation_ = 0.0f;

  float emit_accumulator_ = 0.0f;
  bool is_playing_ = false;
  Rendering::RenderSettings render_settings_;
  SpawnFn spawn_fn_;
  std::function<void()> on_all_dead_;

  thread_local static std::mt19937 rng_;
};
