#include "Scenes/title_scene/title_scene.h"

#include "Asset/asset_manager.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/Renderer/particle_emitter.h"
#include "Component/Renderer/ui_glass_renderer.h"
#include "Component/Renderer/ui_sprite_renderer.h"
#include "Component/Renderer/ui_text_renderer.h"
#include "Component/camera_component.h"
#include "Component/transform_component.h"
#include "Framework/Input/input.h"
#include "Graphic/graphic.h"
#include "Math/Math.h"
#include "ProceduralTexture/procedural_texture_factory.h"
#include "game_context.h"
#include "game_object.h"
#include "scene_id.h"
#include "scene_manager.h"

namespace {

constexpr float DESIGN_HEIGHT = 1080.0f;
constexpr float UI_SCALE = 1.5f;

constexpr float LOGO_W = 480.0f;
constexpr float LOGO_H = 270.0f;
constexpr float BUTTON_W = 200.0f;
constexpr float BUTTON_H = 50.0f;
constexpr float BUTTON_GAP = 16.0f;
constexpr float PADDING = 24.0f;
constexpr float SPRITE_PADDING = 16.0f;
constexpr float TEXT_SIZE = 28.0f;

constexpr float RANDOM_DIRECTION_RANGE = 0.2f;
constexpr float PARALLAX_MAX_ANGLE = 12.0f;
constexpr float PARALLAX_SMOOTHNESS = 3.0f;
ParticleEmitter::SpawnFn SpawnEnvironmentParticle() {
  return [](std::mt19937& rng) -> ParticleEmitter::SpawnParams {
    std::uniform_real_distribution<float> angle_dist(0.0f, Math::TwoPi);
    std::uniform_real_distribution<float> radius_dist(0.0f, 1.0f);
    std::uniform_real_distribution<float> x_offset(-RANDOM_DIRECTION_RANGE, RANDOM_DIRECTION_RANGE);
    std::uniform_real_distribution<float> y_offset(-RANDOM_DIRECTION_RANGE, RANDOM_DIRECTION_RANGE);
    std::uniform_real_distribution<float> z_offset(-RANDOM_DIRECTION_RANGE, RANDOM_DIRECTION_RANGE);

    float angle = angle_dist(rng);
    float r = std::sqrt(radius_dist(rng));

    float x = r * std::cos(angle);
    float z = r * std::sin(angle);

    return {
      .offset = {x, 0.0f, z},
      .direction = Vector3{x_offset(rng), 1.0f, z_offset(rng)}.Normalized(),
    };
  };
}
}  // namespace

void TitleScene::OnEnter(AssetManager& asset_manager) {
  SetSceneName("Title");
  input_ = GetContext()->GetInput();

  GetBackgroundSetting().SetSkybox("Content/skybox/sunflowers_puresky_standard_cubemap_4k.hdr", asset_manager);

  auto* camera_obj = CreateGameObject("TitleCamera");
  auto* camera = camera_obj->AddComponent<CameraComponent>();
  camera_transform_ = camera_obj->GetTransform();
  GetCameraSetting().Register(camera);

  auto procedural_pixels = GenerateProceduralTexture({.size = 64, .falloff = 10.0f, .shape = ProceduralShape::Circle});
  asset_manager.CreateTextureFromPixels("procedural:circle_64", procedural_pixels.data(), 64, 64);

  auto* blue_go = CreateGameObject("Title_Particles_Blue", {.position = {0.0f, 0.0f, 12.0f}});
  auto* blue_emitter = blue_go->AddComponent<ParticleEmitter>(ParticleEmitter::Props{
    .texture_path = "procedural:circle_64",
    .max_particles = 1500,
    .emit_rate = 200.0f,
    .particle_lifetime = 10.0f,
    .particle_size = {0.15f, 0.15f},
    .start_color = {0.2f, 0.5f, 1.0f, 1.0f},
    .end_color = {0.1f, 0.3f, 0.9f, 0.0f},
    .start_speed = 0.4f,
    .speed_variation = 0.2f,
    .gravity = {0.0f, 0.0f, 0.0f},
    .loop = true,
    .blend_mode = Rendering::BlendMode::Additive,
    .spawn_offset = {0.0f, -2.0f, 2.0f},
    .spawn_shape = SpawnShape::Custom,
    .spawn_radius = 10.0f,
    .fade_in_ratio = 0.1f,
    .fade_out_ratio = 0.4f,
    .emissive_intensity = 10.0f,
    .spawn_fn = SpawnEnvironmentParticle(),
  });
  blue_emitter->Play();

  auto* red_go = CreateGameObject("Title_Particles_Red", {.position = {0.0f, 0.0f, 6.0f}});
  auto* red_emitter = red_go->AddComponent<ParticleEmitter>(ParticleEmitter::Props{
    .texture_path = "procedural:circle_64",
    .max_particles = 1000,
    .emit_rate = 100.0f,
    .particle_lifetime = 10.0f,
    .particle_size = {0.2f, 0.2f},
    .start_color = {1.0f, 0.3f, 0.1f, 0.9f},
    .end_color = {0.8f, 0.1f, 0.0f, 0.0f},
    .start_speed = 0.5f,
    .speed_variation = 0.2f,
    .gravity = {0.0f, 0.0f, 0.0f},
    .loop = true,
    .blend_mode = Rendering::BlendMode::Additive,
    .spawn_offset = {0.0f, -2.0f, 2.0f},
    .spawn_shape = SpawnShape::Custom,
    .spawn_radius = 5.0f,
    .fade_in_ratio = 0.1f,
    .fade_out_ratio = 0.4f,
    .emissive_intensity = 10.0f,
    .spawn_fn = SpawnEnvironmentParticle(),
  });
  red_emitter->Play();

  auto* root = CreateGameObject("Title_Root");

  logo_panel_ = CreateGameObject("Title_LogoPanel");
  logo_panel_->SetParent(root);
  logo_glass_ = logo_panel_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .layer_id = 1,
  });

  logo_sprite_go_ = CreateGameObject("Title_LogoSprite");
  logo_sprite_go_->SetParent(logo_panel_);
  logo_sprite_ = logo_sprite_go_->AddComponent<UISpriteRenderer>(UISpriteRenderer::Props{
    .texture_path = "Content/textures/title_1.png",
  });
  logo_sprite_->SetUVScale({1, -1});

  start_btn_ = CreateGameObject("Title_StartBtn");
  start_btn_->SetParent(root);
  start_glass_ = start_btn_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .layer_id = 1,
  });

  start_label_go_ = CreateGameObject("Title_StartLabel");
  start_label_go_->SetParent(start_btn_);
  start_label_ = start_label_go_->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"Start",
    .pixel_size = TEXT_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  leave_btn_ = CreateGameObject("Title_LeaveBtn");
  leave_btn_->SetParent(root);
  leave_glass_ = leave_btn_->AddComponent<UIGlassRenderer>(UIGlassRenderer::Props{
    .layer_id = 1,
  });

  leave_label_go_ = CreateGameObject("Title_LeaveLabel");
  leave_label_go_->SetParent(leave_btn_);
  leave_label_ = leave_label_go_->AddComponent<UITextRenderer>(UITextRenderer::Props{
    .text = L"Leave",
    .pixel_size = TEXT_SIZE,
    .h_align = Text::HorizontalAlign::Center,
    .pivot = {0.5f, 0.0f},
  });

  UpdateLayout();
}

void TitleScene::OnExit() {
}

void TitleScene::OnPreUpdate(float dt) {
  auto [mx, my] = input_->GetMousePosition();

  auto* graphic = GetContext()->GetGraphic();
  float screen_w = static_cast<float>(graphic->GetFrameBufferWidth());
  float screen_h = static_cast<float>(graphic->GetFrameBufferHeight());
  float norm_x = (mx / screen_w) - 0.5f;
  float norm_y = (my / screen_h) - 0.5f;

  Vector3 current_euler = camera_transform_->GetRotationDegrees();
  Vector3 target_euler = {norm_y * -PARALLAX_MAX_ANGLE, norm_x * -PARALLAX_MAX_ANGLE, 0.0f};
  float smooth_t = 1.0f - std::exp(-PARALLAX_SMOOTHNESS * dt);
  camera_transform_->SetRotationEulerDegree(Vector3::Lerp(current_euler, target_euler, smooth_t));

  auto hit_test = [](float mx, float my, const Rect& r) { return mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h; };

  static const Math::Vector4 HOVER_TINT = {0.3f, 0.5f, 1.0f, 0.2f};
  static const Math::Vector4 DEFAULT_TINT = {1.0f, 1.0f, 1.0f, 0.1f};

  bool over_start = hit_test(mx, my, start_rect_);
  bool over_leave = hit_test(mx, my, leave_rect_);

  start_glass_->SetTintColor(over_start ? HOVER_TINT : DEFAULT_TINT);
  leave_glass_->SetTintColor(over_leave ? HOVER_TINT : DEFAULT_TINT);

  if (input_->GetMouseButtonDown(Mouse::Button::Left)) {
    if (over_start) {
      GetContext()->GetSceneManager()->RequestLoad(SceneId::CITY_SCENE);
    } else if (over_leave) {
      GetContext()->RequestQuit();
    }
  }
}

void TitleScene::OnRender(FramePacket& /*packet*/) {
  UpdateLayout();
}

void TitleScene::UpdateLayout() {
  auto* graphic = GetContext()->GetGraphic();
  float screen_w = static_cast<float>(graphic->GetFrameBufferWidth());
  float screen_h = static_cast<float>(graphic->GetFrameBufferHeight());
  float s = (screen_h / DESIGN_HEIGHT) * UI_SCALE;

  float logo_w = LOGO_W * s;
  float logo_h = LOGO_H * s;
  float btn_w = BUTTON_W * s;
  float btn_h = BUTTON_H * s;
  float gap = BUTTON_GAP * s;
  float pad = PADDING * s;

  float total_btn_row_w = btn_w * 2.0f + gap;
  float block_w = (std::max)(logo_w, total_btn_row_w);
  float block_h = logo_h + pad + btn_h;

  float block_x = (screen_w - block_w) / 2.0f;
  float block_y = (screen_h - block_h) / 2.0f;

  float logo_x = block_x + (block_w - logo_w) / 2.0f;
  float logo_y = block_y;

  float btn_row_x = block_x + (block_w - total_btn_row_w) / 2.0f;
  float btn_y = logo_y + logo_h + pad;

  auto set_pos = [](GameObject* go, float x, float y) { go->GetTransform()->SetPosition({x, y, 0.0f}); };

  set_pos(logo_panel_, logo_x, logo_y);
  logo_glass_->SetSize({logo_w, logo_h});

  float sprite_pad = SPRITE_PADDING * s;
  float sprite_w = logo_w - sprite_pad * 2.0f;
  float sprite_h = logo_h - sprite_pad * 2.0f;
  set_pos(logo_sprite_go_, sprite_pad, sprite_pad);
  logo_sprite_->SetSize({sprite_w, sprite_h});

  set_pos(start_btn_, btn_row_x, btn_y);
  start_glass_->SetSize({btn_w, btn_h});
  set_pos(start_label_go_, btn_w / 2.0f, (btn_h - TEXT_SIZE * s) / 2.0f);
  start_label_->SetPixelSize(TEXT_SIZE * s);
  start_rect_ = {btn_row_x, btn_y, btn_w, btn_h};

  float leave_x = btn_row_x + btn_w + gap;
  set_pos(leave_btn_, leave_x, btn_y);
  leave_glass_->SetSize({btn_w, btn_h});
  set_pos(leave_label_go_, btn_w / 2.0f, (btn_h - TEXT_SIZE * s) / 2.0f);
  leave_label_->SetPixelSize(TEXT_SIZE * s);
  leave_rect_ = {leave_x, btn_y, btn_w, btn_h};
}
