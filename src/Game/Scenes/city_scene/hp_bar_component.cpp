#include "hp_bar_component.h"

#include <algorithm>

#include "Component/Renderer/sprite_renderer.h"
#include "Scenes/city_scene/city_scene_config.h"
#include "Scenes/city_scene/health_component.h"
#include "game_object.h"
#include "scene.h"

namespace cfg = CitySceneConfig;

HpBarComponent::HpBarComponent(GameObject* owner, const Props& props)
    : BehaviorComponent(owner), bar_width_(props.bar_width), y_offset_(props.y_offset) {
}

void HpBarComponent::OnStart() {
  health_ = GetOwner()->GetComponent<HealthComponent>();
  max_hp_ = health_->GetMaxHP();

  auto* scene = GetOwner()->GetScene();

  constexpr cfg::HpBarConfig HP_BAR;
  float bar_height = bar_width_ * (HP_BAR.fill_px_h / HP_BAR.fill_px_w);
  fill_width_ = bar_width_;
  fill_height_ = bar_height;

  auto create_layer = [&](const std::string& suffix, const char* texture_path, float y_extra = 0.0f) -> SpriteRenderer* {
    auto* go = scene->CreateGameObject(GetOwner()->GetName() + "_" + suffix);
    go->SetTransient(true);
    go->SetParent(GetOwner());
    go->GetTransform()->Apply({.position = {0.0f, y_offset_ + y_extra, 0.0f}});

    return go->AddComponent<SpriteRenderer>(SpriteRenderer::Props{
      .texture_path = texture_path,
      .size = {bar_width_, bar_height},
      .billboard_mode = Billboard::Mode::Spherical,
      .blend_mode = Rendering::BlendMode::AlphaBlend,
    });
  };

  create_layer("HpBack", HP_BAR.texture_back);
  main_renderer_ = create_layer("HpMain", HP_BAR.texture_main, 0.001f);
}

void HpBarComponent::OnUpdate(float /*dt*/) {
  float hp_ratio = (std::max)(health_->GetHP() / max_hp_, 0.01f);
  hp_ratio = (std::min)(hp_ratio, 1.0f);

  main_renderer_->SetSize({fill_width_ * hp_ratio, fill_height_});
  main_renderer_->SetPivot({1.0f - 1.0f / (2.0f * hp_ratio), 0.5f});
  main_renderer_->SetUVOffset({1.0f - hp_ratio, 0.0f});
  main_renderer_->SetUVScale({hp_ratio, 1.0f});
}
