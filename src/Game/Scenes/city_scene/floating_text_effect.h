#pragma once

#include <string>

#include "Component/Renderer/text_renderer.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/explosion_effect.h"
#include "Scenes/city_scene/floating_text_component.h"
#include "game_object.h"
#include "scene.h"

namespace CitySceneEffect {

inline void SpawnFloatingText(IScene* scene, const Math::Vector3& position, const std::wstring& text,
                              const Math::Vector4& color, const CitySceneConfig::FloatingTextConfig& cfg) {
  constexpr float FONT_PIXEL_SIZE = 48.0f;
  float s = cfg.world_height / FONT_PIXEL_SIZE;

  auto* go = scene->CreateGameObject(MakeUniqueEffectName("FloatingText"), {.position = position, .scale = {s, s, s}});
  go->SetTransient(true);

  auto* text_renderer = go->AddComponent<TextRenderer>(TextRenderer::Props{
    .text = text,
    .pixel_size = FONT_PIXEL_SIZE,
    .color = color,
    .h_align = Text::HorizontalAlign::Center,
    .billboard_mode = Billboard::Mode::Spherical,
    .double_sided = true,
  });
  text_renderer->SetBlendMode(Rendering::BlendMode::Additive);

  go->AddComponent<FloatingTextComponent>(FloatingTextComponent::Props{
    .speed = cfg.speed,
    .visible_duration = cfg.visible_duration,
    .fade_duration = cfg.fade_duration,
    .color = color,
  });
}

inline void SpawnDamageText(IScene* scene, const Math::Vector3& position, float damage) {
  const CitySceneConfig::FloatingTextConfig cfg;
  int rounded = static_cast<int>(damage);
  std::wstring text = L"-" + std::to_wstring(rounded);
  SpawnFloatingText(scene, position, text, cfg.damage_color, cfg);
}

inline void SpawnRewardText(IScene* scene, const Math::Vector3& position, int amount) {
  const CitySceneConfig::FloatingTextConfig cfg;
  std::wstring text = L"+" + std::to_wstring(amount);
  SpawnFloatingText(scene, position, text, cfg.reward_color, cfg);
}

inline void SpawnCostText(IScene* scene, const Math::Vector3& position, int cost) {
  const CitySceneConfig::FloatingTextConfig cfg;
  std::wstring text = L"-" + std::to_wstring(cost);
  SpawnFloatingText(scene, position, text, cfg.reward_color, cfg);
}

inline void SpawnWarningText(IScene* scene, const Math::Vector3& position, const std::wstring& text) {
  const CitySceneConfig::FloatingTextConfig cfg;
  SpawnFloatingText(scene, position, text, cfg.warning_color, cfg);
}

}  // namespace CitySceneEffect
