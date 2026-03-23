#pragma once

#include <string>

#include "Component/Renderer/particle_emitter.h"
#include "Component/Renderer/sprite_renderer.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "game_object.h"
#include "scene.h"

namespace CitySceneEffect {

inline std::string MakeUniqueEffectName(const char* prefix) {
  static int counter = 0;
  return std::string(prefix) + "_" + std::to_string(counter++);
}

struct ExplosionParams {
  const char* texture;
  DirectX::XMUINT2 sheet_size;
  DirectX::XMUINT2 frame_size;
  uint32_t start_row;
  uint32_t frames_per_row;
  uint32_t frame_count;
  float fps;
  float sprite_size;
  Math::Vector3 emissive_color;
  float emissive_intensity;
};

inline ExplosionParams FromArrivalConfig(const CitySceneConfig::ArrivalExplosionConfig& cfg) {
  return {cfg.texture,
    cfg.sheet_size,
    cfg.frame_size,
    cfg.start_row,
    cfg.frames_per_row,
    cfg.frame_count,
    cfg.fps,
    cfg.sprite_size,
    cfg.emissive_color,
    cfg.emissive_intensity};
}

inline ExplosionParams FromBulletHitConfig(const CitySceneConfig::BulletHitConfig& cfg) {
  return {cfg.texture,
    cfg.sheet_size,
    cfg.frame_size,
    cfg.start_row,
    cfg.frames_per_row,
    cfg.frame_count,
    cfg.fps,
    cfg.sprite_size,
    cfg.emissive_color,
    cfg.emissive_intensity};
}

inline void SpawnExplosion(IScene* scene, const Math::Vector3& position, const ExplosionParams& p, const char* name = "Explosion") {
  auto* effect_go = scene->CreateGameObject(MakeUniqueEffectName(name), {.position = position});
  effect_go->SetTransient(true);

  auto c = p.emissive_color * p.emissive_intensity;
  auto* sprite = effect_go->AddComponent<SpriteRenderer>(SpriteRenderer::Props{
    .texture_path = p.texture,
    .color = {c.x, c.y, c.z, 1.0f},
    .size = {p.sprite_size, p.sprite_size},
    .billboard_mode = Billboard::Mode::Spherical,
    .blend_mode = Rendering::BlendMode::Additive,
    .pivot = {0.5f, 0.5f},
    .double_sided = true,
  });

  auto& animator = sprite->GetAnimator();
  animator.SetSpriteSheetConfig({
    .sheet_size = p.sheet_size,
    .frame_size = p.frame_size,
    .orientation = SpriteSheet::Orientation::Horizontal,
  });
  animator.SetStartFrameOffset(p.start_row * p.frames_per_row);
  animator.SetFrameCount(p.frame_count);
  animator.SetFramesPerSecond(p.fps);
  animator.SetLoopEnabled(false);
  animator.SetAnimationEndCallback([effect_go]() { effect_go->Destroy(); });
  animator.Play();

  auto initial_uv = animator.GetCurrentUV();
  sprite->SetUVOffset(initial_uv.uv_offset);
  sprite->SetUVScale(initial_uv.uv_scale);
}

struct SparksParams {
  const char* texture;
  uint32_t burst_count;
  float lifetime;
  Math::Vector2 size;
  Math::Vector4 start_color;
  Math::Vector4 end_color;
  float start_speed;
  float speed_variation;
  Math::Vector3 gravity;
  float drag;
  float end_size;
  float lifetime_variation;
  float size_variation;
  float emissive_intensity;
  float spawn_radius;
};

inline SparksParams FromExplosionSparksConfig(const CitySceneConfig::ExplosionSparksConfig& cfg) {
  return {cfg.texture, cfg.burst_count, cfg.lifetime, cfg.size, cfg.start_color, cfg.end_color,
    cfg.start_speed, cfg.speed_variation, cfg.gravity, cfg.drag, cfg.end_size,
    cfg.lifetime_variation, cfg.size_variation, cfg.emissive_intensity, cfg.spawn_radius};
}

inline ExplosionParams FromBaseDestroyedConfig(const CitySceneConfig::BaseDestroyedExplosionConfig& cfg) {
  return {cfg.texture, cfg.sheet_size, cfg.frame_size, cfg.start_row, cfg.frames_per_row,
    cfg.frame_count, cfg.fps, cfg.sprite_size, cfg.emissive_color, cfg.emissive_intensity};
}

inline SparksParams FromBaseDestroyedSparksConfig(const CitySceneConfig::BaseDestroyedSparksConfig& cfg) {
  return {cfg.texture, cfg.burst_count, cfg.lifetime, cfg.size, cfg.start_color, cfg.end_color,
    cfg.start_speed, cfg.speed_variation, cfg.gravity, cfg.drag, cfg.end_size,
    cfg.lifetime_variation, cfg.size_variation, cfg.emissive_intensity, cfg.spawn_radius};
}

inline SparksParams FromBulletHitSparksConfig(const CitySceneConfig::BulletHitSparksConfig& cfg) {
  return {cfg.texture, cfg.burst_count, cfg.lifetime, cfg.size, cfg.start_color, cfg.end_color,
    cfg.start_speed, cfg.speed_variation, cfg.gravity, cfg.drag, cfg.end_size,
    cfg.lifetime_variation, cfg.size_variation, cfg.emissive_intensity, cfg.spawn_radius};
}

inline void SpawnExplosionSparks(IScene* scene, const Math::Vector3& position, const SparksParams& p, const char* name = "ExplosionSparks") {
  auto* effect_go = scene->CreateGameObject(MakeUniqueEffectName(name), {.position = position});
  effect_go->SetTransient(true);

  auto* emitter = effect_go->AddComponent<ParticleEmitter>(ParticleEmitter::Props{
    .texture_path = p.texture,
    .max_particles = p.burst_count,
    .particle_lifetime = p.lifetime,
    .particle_size = p.size,
    .start_color = p.start_color,
    .end_color = p.end_color,
    .start_speed = p.start_speed,
    .speed_variation = p.speed_variation,
    .gravity = p.gravity,
    .loop = false,
    .burst_count = p.burst_count,
    .drag = p.drag,
    .end_size = p.end_size,
    .lifetime_variation = p.lifetime_variation,
    .size_variation = p.size_variation,
    .spawn_radius = p.spawn_radius,
    .emissive_intensity = p.emissive_intensity,
  });

  emitter->SetOnAllDeadCallback([effect_go]() { effect_go->Destroy(); });
  emitter->Play();
}

}  // namespace CitySceneEffect
