#pragma once

#include <functional>
#include <random>
#include <vector>

#include "Asset/asset_handle.h"
#include "Component/component.h"
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

class ParticleEmitter : public Component<ParticleEmitter> {
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
    Rendering::BlendMode blend_mode = Rendering::BlendMode::Additive;
    SpawnFn spawn_fn = SpawnFromCenter();
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
  }

  void Play();
  void Stop();
  void Clear();

  bool IsPlaying() const {
    return is_playing_;
  }

  void OnUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;

  static SpawnFn SpawnFromCenter();

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

  float emit_accumulator_ = 0.0f;
  bool is_playing_ = false;
  Rendering::RenderSettings render_settings_;
  SpawnFn spawn_fn_;

  thread_local static std::mt19937 rng_;
};
