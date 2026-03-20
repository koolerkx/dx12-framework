#include "Scenes/city_scene/hud_manager_component.h"

#include <cstdio>

#include "Component/Renderer/ui_glass_renderer.h"
#include "Component/Renderer/ui_sprite_renderer.h"
#include "Component/Renderer/ui_text_renderer.h"
#include "Framework/Event/event_bus.hpp"
#include "Framework/Input/input.h"
#include "Scenes/city_scene/city_scene_events.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"
#include "scene_events.h"
#include "scene_id.h"
#include "scene_manager.h"

namespace {

constexpr float DESIGN_HEIGHT = 1080.0f;
constexpr float UI_SCALE = 1.5f;
constexpr float SAFE_AREA = 48.0f;
constexpr float PADDING = 16.0f;
constexpr float TEXT_SIZE = 32.0f;
constexpr float SMALL_TEXT_SIZE = 28.0f;
constexpr float TEXT_LINE_HEIGHT = 36.0f;
constexpr float FADE_SPEED = 5.0f;
constexpr float FADE_EPSILON = 0.001f;

constexpr float HINT_TEXT_GAP = 8.0f;
constexpr float HINT_ICON_GAP = 2.0f;
constexpr float HINT_ROW_GAP = 4.0f;
constexpr float HINT_SECTION_GAP = 12.0f;
constexpr float HINT_ICON_PADDING_V = 3.0f;
constexpr float HINT_TIP_TEXT_RATIO = 0.75f;
constexpr float HINT_PANEL_WIDTH = 300.0f;

struct PanelLayout {
  float width;
  float height;
};

constexpr PanelLayout INFO_PANEL = {240.0f, 156.0f};
constexpr PanelLayout MESSAGE_PANEL = {480.0f, 68.0f};
constexpr PanelLayout ALERT_PANEL = {360.0f, 52.0f};
constexpr PanelLayout ICON_SLOT = {128.0f, 128.0f};
constexpr PanelLayout CONFIRM_BUTTON = {300.0f, 66.0f};
constexpr float BUTTON_GAP = 12.0f;

constexpr PanelLayout GAMEOVER_PANEL = {480.0f, 120.0f};
constexpr PanelLayout GAMEOVER_BUTTON = {200.0f, 50.0f};
constexpr float GAMEOVER_TITLE_SIZE = 48.0f;
constexpr float GAMEOVER_BUTTON_GAP = 16.0f;

float MoveToward(float current, float target, float max_delta) {
  if (target > current) {
    return (std::min)(current + max_delta, target);
  }
  return (std::max)(current - max_delta, target);
}

}  // namespace

HudManagerComponent::HudManagerComponent(GameObject* owner, const Props& /*props*/) : BehaviorComponent(owner) {
}

void HudManagerComponent::OnInit() {
  auto* scene = static_cast<IScene*>(GetOwner()->GetScene());
  auto* hud_root = GetOwner();

  info_panel_ = scene->CreateGameObject("HUD_Info");
  info_panel_->SetParent(hud_root);
  info_glass_ = info_panel_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {INFO_PANEL.width, INFO_PANEL.height},
    .layer_id = 1,
  });

  auto* wave_go = scene->CreateGameObject("HUD_WaveText");
  wave_go->SetParent(info_panel_);
  wave_text_ = wave_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"ウェーブ 1",
    .pixel_size = TEXT_SIZE,
    .pivot = {0.0f, 0.0f},
  });

  auto* hp_go = scene->CreateGameObject("HUD_HPText");
  hp_go->SetParent(info_panel_);
  hp_text_ = hp_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"HP: 100",
    .pixel_size = TEXT_SIZE,
    .pivot = {0.0f, 0.0f},
  });

  auto* gold_go = scene->CreateGameObject("HUD_GoldText");
  gold_go->SetParent(info_panel_);
  gold_text_ = gold_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"ゴールド: 0",
    .pixel_size = TEXT_SIZE,
    .pivot = {0.0f, 0.0f},
  });

  auto* message_panel = scene->CreateGameObject("HUD_Message");
  message_panel->SetParent(hud_root);
  auto* message_glass = message_panel->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {MESSAGE_PANEL.width, MESSAGE_PANEL.height},
    .layer_id = 1,
  });

  auto* msg_text_go = scene->CreateGameObject("HUD_MessageText");
  msg_text_go->SetParent(message_panel);
  auto* message_text = msg_text_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"",
    .pixel_size = TEXT_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  message_fade_ = {
    .panel = message_panel,
    .glass = message_glass,
    .text = message_text,
    .base_tint_alpha = 0.15f,
  };
  message_panel->SetActive(false);

  auto* alert_panel = scene->CreateGameObject("HUD_Alert");
  alert_panel->SetParent(hud_root);
  auto* alert_glass = alert_panel->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {ALERT_PANEL.width, ALERT_PANEL.height},
    .tint_color = {1.0f, 0.0f, 0.0f, 0.3f},
    .layer_id = 1,
  });

  auto* alert_text_go = scene->CreateGameObject("HUD_AlertText");
  alert_text_go->SetParent(alert_panel);
  auto* alert_text = alert_text_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"",
    .pixel_size = SMALL_TEXT_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  alert_fade_ = {
    .panel = alert_panel,
    .glass = alert_glass,
    .text = alert_text,
    .base_tint_alpha = 0.15f,
  };
  alert_panel->SetActive(false);

  hint_panel_ = scene->CreateGameObject("HUD_Hint");
  hint_panel_->SetParent(hud_root);
  hint_glass_ = hint_panel_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {100.0f, 100.0f},
    .layer_id = 1,
  });
  BuildHintPanel();

  auto* icon_go = scene->CreateGameObject("HUD_IconSlot_0");
  icon_go->SetParent(hud_root);
  auto* icon_glass = icon_go->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {ICON_SLOT.width, ICON_SLOT.height},
    .layer_id = 1,
  });

  auto* icon_sprite_go = scene->CreateGameObject("HUD_IconSprite_0");
  icon_sprite_go->SetParent(icon_go);
  auto* icon_sprite = icon_sprite_go->AddComponent<UISpriteRenderer>(UISpriteRenderer::Props{
    .texture_path = "Content/textures/tower-square-build-d.png",
    .size = {ICON_SLOT.width, ICON_SLOT.height},
  });
  icon_sprite->SetUVScale({1.0f, -1.0f});

  icon_slots_.push_back({.root = icon_go, .glass = icon_glass, .icon = icon_sprite});

  confirm_panel_ = scene->CreateGameObject("HUD_ConfirmPanel");
  confirm_panel_->SetParent(hud_root);
  confirm_panel_glass_ = confirm_panel_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {CONFIRM_BUTTON.width, CONFIRM_BUTTON.height},
    .layer_id = 1,
  });

  auto* cost_go = scene->CreateGameObject("HUD_CostText");
  cost_go->SetParent(confirm_panel_);
  cost_text_ = cost_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"コスト: 0",
    .pixel_size = TEXT_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  auto create_button = [&](const char* name, const wchar_t* label_text) -> ButtonSlot {
    ButtonSlot slot;
    slot.root = scene->CreateGameObject(name);
    slot.root->SetParent(hud_root);
    slot.glass = slot.root->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
      .size = {CONFIRM_BUTTON.width, CONFIRM_BUTTON.height},
      .layer_id = 2,
    });
    auto* label_go = scene->CreateGameObject(std::string(name) + "_Label");
    label_go->SetParent(slot.root);
    slot.label = label_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
      .text = label_text,
      .pixel_size = SMALL_TEXT_SIZE,
      .h_align = Text::HorizontalAlign::Center,
      .pivot = {0.5f, 0.0f},
    });
    return slot;
  };

  confirm_button_ = create_button("HUD_ConfirmBtn", L"確認");
  cancel_button_ = create_button("HUD_CancelBtn", L"キャンセル");
  SetConfirmPanelVisible(false);

  gameover_panel_ = scene->CreateGameObject("HUD_GameOverPanel");
  gameover_panel_->SetParent(hud_root);
  gameover_panel_glass_ = gameover_panel_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .size = {GAMEOVER_PANEL.width, GAMEOVER_PANEL.height},
    .layer_id = 3,
  });

  auto* go_title = scene->CreateGameObject("HUD_GameOverTitle");
  go_title->SetParent(gameover_panel_);
  gameover_title_text_ = go_title->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"ゲームオーバー",
    .pixel_size = GAMEOVER_TITLE_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  auto* go_stats = scene->CreateGameObject("HUD_GameOverStats");
  go_stats->SetParent(gameover_panel_);
  gameover_stats_text_ = go_stats->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"",
    .pixel_size = SMALL_TEXT_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  restart_button_ = create_button("HUD_RestartBtn", L"リスタート");
  restart_button_.glass->SetLayerId(3);
  title_button_ = create_button("HUD_TitleBtn", L"タイトルへ戻る");
  title_button_.glass->SetLayerId(3);

  gameover_panel_->SetActive(false);
  restart_button_.root->SetActive(false);
  title_button_.root->SetActive(false);

  gameover_overlay_.Create(scene, "GameOverTransitionOverlay");

  input_ = GetContext()->GetInput();
  SubscribeEvents();
  UpdateLayout();
}

void HudManagerComponent::SubscribeEvents() {
  auto& bus = *GetContext()->GetEventBus();

  event_scope_.Subscribe<GoldChangedEvent>(bus, [this](const GoldChangedEvent& e) { SetGold(e.gold); });

  event_scope_.Subscribe<HealthChangedEvent>(bus, [this](const HealthChangedEvent& e) { SetHealth(e.health); });

  event_scope_.Subscribe<WaveStartEvent>(bus, [this](const WaveStartEvent& e) {
    SetWave(e.wave);
    ShowMessage(L"ウェーブ " + std::to_wstring(e.wave) + L"!", 3.0f);
    message_fade_.is_countdown = false;
  });

  event_scope_.Subscribe<WaveCountdownEvent>(bus, [this](const WaveCountdownEvent& e) {
    wchar_t buf[64];
    swprintf_s(buf, L"次のウェーブまで、あと%.1f秒", e.seconds_remaining);
    ShowCountdownMessage(buf);
  });

  event_scope_.Subscribe<InsufficientGoldEvent>(bus, [this](const InsufficientGoldEvent&) { ShowAlert(L"ゴールドが足りません！", 3.0f); });

  event_scope_.Subscribe<EnemyReachedBaseEvent>(
    bus, [this](const EnemyReachedBaseEvent&) { ShowAlert(L"拠点が攻撃されています！", 3.0f); });

  event_scope_.Subscribe<OverlapEnemyEvent>(bus, [this](const OverlapEnemyEvent&) { ShowAlert(L"敵と重なっています！", 3.0f); });

  event_scope_.Subscribe<OverlapEnemySpawnEvent>(
    bus, [this](const OverlapEnemySpawnEvent&) { ShowAlert(L"敵の出現地点と重なっています！", 3.0f); });

  event_scope_.Subscribe<TowerPlacementExitedEvent>(bus, [this](const TowerPlacementExitedEvent&) {
    icon_state_ = IconState::Normal;
    if (!icon_slots_.empty()) {
      icon_slots_[0].glass->SetTintColor({1.0f, 1.0f, 1.0f, 0.1f});
    }
    SetConfirmPanelVisible(false);
    SetHintMode(HintMode::Normal);
  });

  event_scope_.Subscribe<TowerPlacementSelectedEvent>(bus, [this](const TowerPlacementSelectedEvent& e) {
    cost_text_->SetText(L"コスト: " + std::to_wstring(e.cost));
    SetConfirmPanelVisible(true);
    SetHintMode(HintMode::ConfirmingTower);
  });

  event_scope_.Subscribe<TowerPlacementCancelledEvent>(bus, [this](const TowerPlacementCancelledEvent&) {
    SetConfirmPanelVisible(false);
    SetHintMode(HintMode::PlacingTower);
  });

  event_scope_.Subscribe<GameOverEvent>(bus, [this](const GameOverEvent& e) {
    SetGameplayHudVisible(false);

    wchar_t buf[128];
    swprintf_s(buf, L"ウェーブ: %d  |  撃破数: %d", e.wave, e.kill_count);
    gameover_stats_text_->SetText(buf);

    gameover_panel_->SetActive(true);
    restart_button_.root->SetActive(true);
    title_button_.root->SetActive(true);
    gameover_active_ = true;
  });
}

void HudManagerComponent::OnUpdate(float dt) {
  gameover_overlay_.Update(dt);
  UpdateFadePanel(message_fade_, dt);
  UpdateFadePanel(alert_fade_, dt);

  UpdateLayout();

  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  float screen_w = static_cast<float>(graphic->GetSceneWidth());
  float screen_h = static_cast<float>(graphic->GetSceneHeight());
  gameover_overlay_.UpdateLayout(screen_w, screen_h);

  if (gameover_active_) {
    if (!gameover_overlay_.IsFadingIn()) {
      UpdateGameOverInteraction();
    }
    return;
  }
  UpdateIconInteraction();
  UpdateConfirmPanelInteraction();
}

void HudManagerComponent::UpdateFadePanel(FadePanel& fade, float dt) {
  if (fade.auto_hide_delay >= 0.0f) {
    fade.auto_hide_delay -= dt;
    if (fade.auto_hide_delay <= 0.0f) {
      fade.target_opacity = 0.0f;
      fade.auto_hide_delay = -1.0f;
    }
  }

  if (fade.opacity == fade.target_opacity) return;

  fade.opacity = MoveToward(fade.opacity, fade.target_opacity, FADE_SPEED * dt);

  if (fade.opacity < FADE_EPSILON) {
    fade.opacity = 0.0f;
    fade.panel->SetActive(false);
    return;
  }

  fade.text->SetColor({1.0f, 1.0f, 1.0f, fade.opacity});
  fade.glass->SetTintAlpha(fade.base_tint_alpha * fade.opacity);
}

void HudManagerComponent::UpdateLayout() {
  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  float screen_w = static_cast<float>(graphic->GetSceneWidth());
  float screen_h = static_cast<float>(graphic->GetSceneHeight());
  float s = (screen_h / DESIGN_HEIGHT) * UI_SCALE;

  auto set_pos = [](GameObject* go, float x, float y) { go->GetTransform()->SetPosition({x, y, 0.0f}); };

  set_pos(info_panel_, SAFE_AREA * s, SAFE_AREA * s);
  info_glass_->SetSize({INFO_PANEL.width * s, INFO_PANEL.height * s});

  set_pos(wave_text_->GetOwner(), PADDING * s, PADDING * s);
  wave_text_->SetPixelSize(TEXT_SIZE * s);

  set_pos(hp_text_->GetOwner(), PADDING * s, (PADDING + TEXT_LINE_HEIGHT) * s);
  hp_text_->SetPixelSize(TEXT_SIZE * s);

  set_pos(gold_text_->GetOwner(), PADDING * s, (PADDING + TEXT_LINE_HEIGHT * 2.0f) * s);
  gold_text_->SetPixelSize(TEXT_SIZE * s);

  float msg_x = (screen_w - MESSAGE_PANEL.width * s) / 2.0f;
  set_pos(message_fade_.panel, msg_x, SAFE_AREA * s);
  message_fade_.glass->SetSize({MESSAGE_PANEL.width * s, MESSAGE_PANEL.height * s});

  set_pos(message_fade_.text->GetOwner(), MESSAGE_PANEL.width * s / 2.0f, PADDING * s);
  message_fade_.text->SetPixelSize(TEXT_SIZE * s);

  float alert_y = (SAFE_AREA + MESSAGE_PANEL.height + 12.0f) * s;
  float alert_x = (screen_w - ALERT_PANEL.width * s) / 2.0f;
  set_pos(alert_fade_.panel, alert_x, alert_y);
  alert_fade_.glass->SetSize({ALERT_PANEL.width * s, ALERT_PANEL.height * s});

  set_pos(alert_fade_.text->GetOwner(), ALERT_PANEL.width * s / 2.0f, PADDING * s);
  alert_fade_.text->SetPixelSize(SMALL_TEXT_SIZE * s);

  float hint_text_px = SMALL_TEXT_SIZE * s;
  float hint_icon_sz = (TEXT_LINE_HEIGHT - HINT_ICON_PADDING_V * 2.0f) * s;
  float hint_text_gap = HINT_TEXT_GAP * s;
  float hint_icon_gap = HINT_ICON_GAP * s;
  float hint_pad = PADDING * s;

  std::vector<HintRow*> visible_hints;
  visible_hints.reserve(common_hint_rows_.size() + 1);
  for (auto& row : common_hint_rows_)
    visible_hints.push_back(&row);
  HintRow* active_state_row = &normal_hint_row_;
  if (hint_mode_ == HintMode::PlacingTower) active_state_row = &placing_tower_hint_row_;
  if (hint_mode_ == HintMode::ConfirmingTower) active_state_row = &confirming_tower_hint_row_;
  visible_hints.push_back(active_state_row);

  float hint_tip_px = hint_text_px * HINT_TIP_TEXT_RATIO;

  std::vector<float> hint_row_h(visible_hints.size());
  for (size_t i = 0; i < visible_hints.size(); ++i) {
    auto* row = visible_hints[i];
    float row_text_px = row->icons.empty() ? hint_tip_px : hint_text_px;
    row->text->SetPixelSize(row_text_px);
    auto tsz = row->text->GetSize();
    hint_row_h[i] = row->icons.empty() ? (std::max)(TEXT_LINE_HEIGHT * s, tsz.y) : TEXT_LINE_HEIGHT * s;
  }

  float hint_panel_w = HINT_PANEL_WIDTH * s;
  float hint_panel_h = hint_pad * 2.0f;
  for (size_t i = 0; i < visible_hints.size(); ++i) {
    hint_panel_h += hint_row_h[i];
    if (i < visible_hints.size() - 1) {
      hint_panel_h += (i == common_hint_rows_.size() - 1) ? HINT_SECTION_GAP * s : HINT_ROW_GAP * s;
    }
  }

  float hint_x = screen_w - SAFE_AREA * s - hint_panel_w;
  set_pos(hint_panel_, hint_x, SAFE_AREA * s);
  hint_glass_->SetSize({hint_panel_w, hint_panel_h});

  float hint_cy = hint_pad;
  for (size_t i = 0; i < visible_hints.size(); ++i) {
    auto* row = visible_hints[i];
    set_pos(row->root, 0.0f, hint_cy);

    float row_text_px = row->icons.empty() ? hint_tip_px : hint_text_px;
    float ty = row->icons.empty() ? 0.0f : (hint_row_h[i] - row_text_px) * 0.5f;
    set_pos(row->text->GetOwner(), hint_pad, ty);

    if (!row->icons.empty()) {
      float tw = row->text->GetSize().x;
      float ix = hint_pad + tw + hint_text_gap;
      float iy = (hint_row_h[i] - hint_icon_sz) * 0.5f;
      for (auto* sprite : row->icons) {
        set_pos(sprite->GetOwner(), ix, iy);
        sprite->SetSize({hint_icon_sz, hint_icon_sz});
        ix += hint_icon_sz + hint_icon_gap;
      }
    }

    hint_cy += hint_row_h[i];
    if (i < visible_hints.size() - 1) {
      hint_cy += (i == common_hint_rows_.size() - 1) ? HINT_SECTION_GAP * s : HINT_ROW_GAP * s;
    }
  }

  panel_rects_.clear();
  panel_rects_.push_back({SAFE_AREA * s, SAFE_AREA * s, INFO_PANEL.width * s, INFO_PANEL.height * s});
  panel_rects_.push_back({hint_x, SAFE_AREA * s, hint_panel_w, hint_panel_h});

  if (message_fade_.panel->IsActive()) {
    panel_rects_.push_back({msg_x, SAFE_AREA * s, MESSAGE_PANEL.width * s, MESSAGE_PANEL.height * s});
  }
  if (alert_fade_.panel->IsActive()) {
    panel_rects_.push_back({alert_x, alert_y, ALERT_PANEL.width * s, ALERT_PANEL.height * s});
  }

  for (size_t i = 0; i < icon_slots_.size(); ++i) {
    float icon_x = (SAFE_AREA + static_cast<float>(i) * 140.0f) * s;
    float icon_y = screen_h - (SAFE_AREA + ICON_SLOT.height) * s;
    float icon_size = ICON_SLOT.width * s;

    set_pos(icon_slots_[i].root, icon_x, icon_y);
    icon_slots_[i].glass->SetSize({icon_size, icon_size});
    icon_slots_[i].icon->SetSize({icon_size, icon_size});

    panel_rects_.push_back({icon_x, icon_y, icon_size, icon_size});
  }

  if (confirm_panel_->IsActive()) {
    float btn_w = CONFIRM_BUTTON.width * s;
    float btn_h = CONFIRM_BUTTON.height * s;
    float gap = BUTTON_GAP * s;
    float total_h = btn_h * 3.0f + gap * 2.0f;

    float base_x = screen_w - (SAFE_AREA * s + btn_w);
    float base_y = screen_h - (SAFE_AREA * s + total_h);

    set_pos(confirm_panel_, base_x, base_y);
    confirm_panel_glass_->SetSize({btn_w, btn_h});
    set_pos(cost_text_->GetOwner(), btn_w * 0.5f, (btn_h - TEXT_SIZE * s) * 0.5f);
    cost_text_->SetPixelSize(TEXT_SIZE * s);

    float confirm_y = base_y + btn_h + gap;
    set_pos(confirm_button_.root, base_x, confirm_y);
    confirm_button_.glass->SetSize({btn_w, btn_h});
    set_pos(confirm_button_.label->GetOwner(), btn_w * 0.5f, (btn_h - SMALL_TEXT_SIZE * s) * 0.5f);
    confirm_button_.label->SetPixelSize(SMALL_TEXT_SIZE * s);
    confirm_button_.rect = {base_x, confirm_y, btn_w, btn_h};

    float cancel_y = confirm_y + btn_h + gap;
    set_pos(cancel_button_.root, base_x, cancel_y);
    cancel_button_.glass->SetSize({btn_w, btn_h});
    set_pos(cancel_button_.label->GetOwner(), btn_w * 0.5f, (btn_h - SMALL_TEXT_SIZE * s) * 0.5f);
    cancel_button_.label->SetPixelSize(SMALL_TEXT_SIZE * s);
    cancel_button_.rect = {base_x, cancel_y, btn_w, btn_h};

    panel_rects_.push_back({base_x, base_y, btn_w, total_h});
  }

  if (gameover_active_) {
    float go_panel_w = GAMEOVER_PANEL.width * s;
    float go_panel_h = GAMEOVER_PANEL.height * s;
    float go_btn_w = GAMEOVER_BUTTON.width * s;
    float go_btn_h = GAMEOVER_BUTTON.height * s;
    float go_btn_gap = GAMEOVER_BUTTON_GAP * s;
    float gap_between = PADDING * s;
    float total_block_h = go_panel_h + gap_between + go_btn_h;

    float go_panel_x = (screen_w - go_panel_w) / 2.0f;
    float go_panel_y = (screen_h - total_block_h) / 2.0f;

    set_pos(gameover_panel_, go_panel_x, go_panel_y);
    gameover_panel_glass_->SetSize({go_panel_w, go_panel_h});

    set_pos(gameover_title_text_->GetOwner(), go_panel_w / 2.0f, PADDING * s);
    gameover_title_text_->SetPixelSize(GAMEOVER_TITLE_SIZE * s);

    float stats_y = PADDING * s + GAMEOVER_TITLE_SIZE * s + 8.0f * s;
    set_pos(gameover_stats_text_->GetOwner(), go_panel_w / 2.0f, stats_y);
    gameover_stats_text_->SetPixelSize(SMALL_TEXT_SIZE * s);

    float total_btn_w = go_btn_w * 2.0f + go_btn_gap;
    float btn_start_x = (screen_w - total_btn_w) / 2.0f;
    float btn_y = go_panel_y + go_panel_h + gap_between;

    set_pos(restart_button_.root, btn_start_x, btn_y);
    restart_button_.glass->SetSize({go_btn_w, go_btn_h});
    set_pos(restart_button_.label->GetOwner(), go_btn_w / 2.0f, (go_btn_h - SMALL_TEXT_SIZE * s) / 2.0f);
    restart_button_.label->SetPixelSize(SMALL_TEXT_SIZE * s);
    restart_button_.rect = {btn_start_x, btn_y, go_btn_w, go_btn_h};

    float title_btn_x = btn_start_x + go_btn_w + go_btn_gap;
    set_pos(title_button_.root, title_btn_x, btn_y);
    title_button_.glass->SetSize({go_btn_w, go_btn_h});
    set_pos(title_button_.label->GetOwner(), go_btn_w / 2.0f, (go_btn_h - SMALL_TEXT_SIZE * s) / 2.0f);
    title_button_.label->SetPixelSize(SMALL_TEXT_SIZE * s);
    title_button_.rect = {title_btn_x, btn_y, go_btn_w, go_btn_h};

    panel_rects_.push_back({go_panel_x, go_panel_y, go_panel_w, go_panel_h});
    panel_rects_.push_back({btn_start_x, btn_y, total_btn_w, go_btn_h});
  }
}

void HudManagerComponent::SetWave(int wave) {
  wave_ = wave;
  wave_text_->SetText(L"ウェーブ " + std::to_wstring(wave));
}

void HudManagerComponent::SetHealth(int hp) {
  hp_ = hp;
  hp_text_->SetText(L"HP: " + std::to_wstring(hp));
}

void HudManagerComponent::SetGold(int gold) {
  gold_ = gold;
  gold_text_->SetText(L"ゴールド: " + std::to_wstring(gold));
}

void HudManagerComponent::ShowMessage(const std::wstring& text, float duration) {
  message_fade_.text->SetText(text);
  message_fade_.target_opacity = 1.0f;
  message_fade_.auto_hide_delay = duration;
  message_fade_.is_countdown = false;
  if (!message_fade_.panel->IsActive()) {
    message_fade_.panel->SetActive(true);
    message_fade_.opacity = 0.0f;
    message_fade_.text->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
    message_fade_.glass->SetTintAlpha(0.0f);
  }
}

void HudManagerComponent::ShowAlert(const std::wstring& text, float duration) {
  alert_fade_.text->SetText(text);
  alert_fade_.target_opacity = 1.0f;
  alert_fade_.auto_hide_delay = duration;
  alert_fade_.is_countdown = false;
  if (!alert_fade_.panel->IsActive()) {
    alert_fade_.panel->SetActive(true);
    alert_fade_.opacity = 0.0f;
    alert_fade_.text->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
    alert_fade_.glass->SetTintAlpha(0.0f);
  }
}

void HudManagerComponent::ShowCountdownMessage(const std::wstring& text) {
  message_fade_.text->SetText(text);
  message_fade_.target_opacity = 1.0f;
  message_fade_.auto_hide_delay = -1.0f;
  message_fade_.is_countdown = true;
  if (!message_fade_.panel->IsActive()) {
    message_fade_.panel->SetActive(true);
    message_fade_.opacity = 0.0f;
    message_fade_.text->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
    message_fade_.glass->SetTintAlpha(0.0f);
  }
}

void HudManagerComponent::BuildHintPanel() {
  common_hint_rows_.push_back(CreateHintRow("HUD_Hint_Move",
    L"視点移動",
    {
      "Content/input/keyboard/keyboard_w_outline.png",
      "Content/input/keyboard/keyboard_a_outline.png",
      "Content/input/keyboard/keyboard_s_outline.png",
      "Content/input/keyboard/keyboard_d_outline.png",
    }));
  common_hint_rows_.push_back(CreateHintRow("HUD_Hint_Elevation",
    L"視点昇降",
    {
      "Content/input/keyboard/keyboard_q_outline.png",
      "Content/input/keyboard/keyboard_e_outline.png",
    }));
  common_hint_rows_.push_back(CreateHintRow("HUD_Hint_Rotate",
    L"視点回転",
    {
      "Content/input/keyboard/keyboard_arrow_left_outline.png",
      "Content/input/keyboard/keyboard_arrow_up_outline.png",
      "Content/input/keyboard/keyboard_arrow_down_outline.png",
      "Content/input/keyboard/keyboard_arrow_right_outline.png",
    }));

  normal_hint_row_ = CreateHintRow("HUD_Hint_Normal", L"左下のタワーを選択して、\n配置しましょう", {});

  placing_tower_hint_row_ = CreateHintRow("HUD_Hint_Placing", L"マップをクリックし、配置しましょう", {});
  placing_tower_hint_row_.root->SetActive(false);

  confirming_tower_hint_row_ = CreateHintRow("HUD_Hint_Confirming", L"右下のボタンで\n確認しましょう", {});
  confirming_tower_hint_row_.root->SetActive(false);
}

HudManagerComponent::HintRow HudManagerComponent::CreateHintRow(
  const char* name, const wchar_t* label, const std::vector<std::string>& icon_paths) {
  auto* scene = static_cast<IScene*>(GetOwner()->GetScene());

  HintRow row;
  row.root = scene->CreateGameObject(name);
  row.root->SetParent(hint_panel_);

  auto* text_go = scene->CreateGameObject(std::string(name) + "_Text");
  text_go->SetParent(row.root);
  row.text = text_go->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = label,
    .pixel_size = SMALL_TEXT_SIZE,
    .pivot = {0.0f, 0.0f},
  });

  for (size_t i = 0; i < icon_paths.size(); ++i) {
    auto* icon_go = scene->CreateGameObject(std::string(name) + "_Icon_" + std::to_string(i));
    icon_go->SetParent(row.root);
    auto* sprite = icon_go->AddComponent<UISpriteRenderer>(UISpriteRenderer::Props{
      .texture_path = icon_paths[i],
      .size = {32.0f, 32.0f},
    });
    sprite->SetUVScale({1.0f, -1.0f});
    row.icons.push_back(sprite);
  }

  return row;
}

void HudManagerComponent::SetHintMode(HintMode mode) {
  if (hint_mode_ == mode) return;
  hint_mode_ = mode;
  normal_hint_row_.root->SetActive(mode == HintMode::Normal);
  placing_tower_hint_row_.root->SetActive(mode == HintMode::PlacingTower);
  confirming_tower_hint_row_.root->SetActive(mode == HintMode::ConfirmingTower);
}

bool HudManagerComponent::IsMouseOverUI(float mx, float my) const {
  for (const auto& r : panel_rects_) {
    if (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h) {
      return true;
    }
  }
  return false;
}

int HudManagerComponent::HitTestIconSlot(float mx, float my) const {
  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  float screen_h = static_cast<float>(graphic->GetSceneHeight());
  float s = (screen_h / DESIGN_HEIGHT) * UI_SCALE;

  for (size_t i = 0; i < icon_slots_.size(); ++i) {
    float icon_x = (SAFE_AREA + static_cast<float>(i) * 140.0f) * s;
    float icon_y = screen_h - (SAFE_AREA + ICON_SLOT.height) * s;
    float icon_size = ICON_SLOT.width * s;

    if (mx >= icon_x && mx <= icon_x + icon_size && my >= icon_y && my <= icon_y + icon_size) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void HudManagerComponent::UpdateIconInteraction() {
  auto [mx, my] = input_->GetMousePosition();
  int hovered_slot = HitTestIconSlot(mx, my);

  if (input_->GetMouseButtonDown(Mouse::Button::Left) && hovered_slot == 0) {
    if (icon_state_ == IconState::Active) {
      icon_state_ = IconState::Normal;
    } else {
      icon_state_ = IconState::Active;
    }
    SetHintMode(icon_state_ == IconState::Active ? HintMode::PlacingTower : HintMode::Normal);
    GetContext()->GetEventBus()->Emit(ToggleTowerPlacementEvent{});
  } else if (icon_state_ != IconState::Active) {
    icon_state_ = (hovered_slot == 0) ? IconState::Hovered : IconState::Normal;
  }

  if (icon_slots_.empty()) return;

  Math::Vector4 tint;
  switch (icon_state_) {
    case IconState::Hovered:
      tint = {0.3f, 0.5f, 1.0f, 0.2f};
      break;
    case IconState::Active:
      tint = {1.0f, 0.85f, 0.0f, 0.25f};
      break;
    default:
      tint = {1.0f, 1.0f, 1.0f, 0.1f};
      break;
  }
  icon_slots_[0].glass->SetTintColor(tint);
}

void HudManagerComponent::SetConfirmPanelVisible(bool visible) {
  confirm_panel_->SetActive(visible);
  confirm_button_.root->SetActive(visible);
  cancel_button_.root->SetActive(visible);
}

void HudManagerComponent::SetGameplayHudVisible(bool visible) {
  info_panel_->SetActive(visible);
  hint_panel_->SetActive(visible);
  for (auto& slot : icon_slots_) {
    slot.root->SetActive(visible);
  }
  if (!visible) {
    SetConfirmPanelVisible(false);
    message_fade_.panel->SetActive(false);
    message_fade_.target_opacity = 0.0f;
    message_fade_.opacity = 0.0f;
    alert_fade_.panel->SetActive(false);
    alert_fade_.target_opacity = 0.0f;
    alert_fade_.opacity = 0.0f;
  }
}

void HudManagerComponent::UpdateGameOverInteraction() {
  auto [mx, my] = input_->GetMousePosition();

  auto hit_test = [](float mx, float my, const PanelRect& r) { return mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h; };

  static const Math::Vector4 HOVER_TINT = {0.3f, 0.5f, 1.0f, 0.2f};
  static const Math::Vector4 DEFAULT_TINT = {1.0f, 1.0f, 1.0f, 0.1f};

  bool over_restart = hit_test(mx, my, restart_button_.rect);
  bool over_title = hit_test(mx, my, title_button_.rect);

  restart_button_.glass->SetTintColor(over_restart ? HOVER_TINT : DEFAULT_TINT);
  title_button_.glass->SetTintColor(over_title ? HOVER_TINT : DEFAULT_TINT);

  if (input_->GetMouseButtonDown(Mouse::Button::Left)) {
    if (over_restart) {
      gameover_overlay_.FadeIn([this]() { GetContext()->GetEventBus()->Emit(RestartGameEvent{}); });
    } else if (over_title) {
      gameover_overlay_.FadeIn([this]() { GetContext()->GetSceneManager()->RequestLoad(SceneId::TITLE_SCENE); });
    }
  }
}

void HudManagerComponent::UpdateConfirmPanelInteraction() {
  if (!confirm_panel_->IsActive()) return;

  auto [mx, my] = input_->GetMousePosition();

  auto hit_test = [](float mx, float my, const PanelRect& r) { return mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h; };

  static const Math::Vector4 HOVER_TINT = {0.3f, 0.5f, 1.0f, 0.2f};
  static const Math::Vector4 DEFAULT_TINT = {1.0f, 1.0f, 1.0f, 0.1f};

  bool over_confirm = hit_test(mx, my, confirm_button_.rect);
  bool over_cancel = hit_test(mx, my, cancel_button_.rect);

  confirm_button_.glass->SetTintColor(over_confirm ? HOVER_TINT : DEFAULT_TINT);
  cancel_button_.glass->SetTintColor(over_cancel ? HOVER_TINT : DEFAULT_TINT);

  if (input_->GetMouseButtonDown(Mouse::Button::Left)) {
    if (over_confirm) {
      GetContext()->GetEventBus()->Emit(TowerPlacementConfirmedEvent{});
    } else if (over_cancel) {
      GetContext()->GetEventBus()->Emit(TowerPlacementCancelledEvent{});
    }
  }
}
