#include "test_scene.h"

#include <DirectXMath.h>

#include "Asset/asset_manager.h"
#include "Scenes/test_scene/character_mover_component.h"
#include "Component/animated_sprite.h"
#include "Component/free_camera_controller.h"
#include "Component/mesh_renderer.h"
#include "Component/pivot_type.h"
#include "Component/sprite_renderer.h"
#include "Component/sprite_sheet_helper.h"
#include "Component/text_renderer.h"
#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Frame/frame_packet.h"
#include "Framework/Logging/logger.h"
#include "test_scene.h"


static float rotation_angle_ = 0.0f;
static float rotation_speed_ = 15.0f;

void TestScene::OnEnter(AssetManager& asset_manager) {
  texture_background_ = asset_manager.LoadTexture("Content/textures/result_bg_1.png");
  texture_character_ = asset_manager.LoadTexture("Content/textures/ship_J.png");
  texture_ao_ = asset_manager.LoadTexture("Content/textures/metal_plate_ao_1k.png");
  texture_additive_ = asset_manager.LoadTexture("Content/textures/sun_additive.png");

  SetupCamera();

  // Create terrain plane
  terrain_plane_ = CreateGameObject("TerrainPlane");
  auto* plane_transform = terrain_plane_->GetComponent<TransformComponent>();
  plane_transform->SetPosition({0.0f, -5.0f, 0.0f});
  plane_transform->SetScale({20.0f, 1.0f, 20.0f});  // 20x20 size

  auto* plane_renderer = terrain_plane_->AddComponent<MeshRenderer>();
  plane_renderer->SetMesh(asset_manager.GetDefaultMesh(DefaultMesh::Plane));
  plane_renderer->SetTexture(texture_background_.Get());
  plane_renderer->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

  cube_object_ = CreateGameObject("Cube");
  auto* cube_transform = cube_object_->GetComponent<TransformComponent>();
  cube_transform->SetPosition({0.0f, 0.0f, 0.0f});  // At origin
  cube_transform->SetScale({2.0f, 2.0f, 2.0f});     // Scale to 2x2x2

  auto* mesh_renderer = cube_object_->AddComponent<MeshRenderer>();
  mesh_renderer->SetMesh(asset_manager.GetDefaultMesh(DefaultMesh::Cube));
  mesh_renderer->SetTexture(texture_background_.Get());  // Use ship texture for testing
  mesh_renderer->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

  cube_object2_ = CreateGameObject("Cube");
  cube_object2_->SetParent(cube_object_);

  auto* cube_transform2 = cube_object2_->GetComponent<TransformComponent>();
  cube_transform2->SetPosition({1.0f, 1.0f, 1.0f});  // At origin
  cube_transform2->SetScale({1.0f, 1.0f, 1.0f});     // Scale to 2x2x2

  auto* mesh_renderer2 = cube_object2_->AddComponent<MeshRenderer>();
  mesh_renderer2->SetMesh(asset_manager.GetDefaultMesh(DefaultMesh::Cube));
  mesh_renderer2->SetTexture(texture_background_.Get());  // Use ship texture for testing
  mesh_renderer2->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

  // auto* background = CreateGameObject("Background");
  // background->GetComponent<TransformComponent>()->SetPosition({500, 500, 0});
  // auto* bg_sprite = background->AddComponent<SpriteRenderer>();
  // bg_sprite->SetTexture(texture_background_.Get());
  // bg_sprite->SetSize({1920, 1080});

  character_object_ = CreateGameObject("Character");
  // character_object_->SetParent(background);
  character_object_->GetComponent<TransformComponent>()->SetPosition({500, 500, 0});
  auto* char_sprite = character_object_->AddComponent<SpriteRenderer>();
  char_sprite->SetTexture(texture_character_.Get());
  char_sprite->SetSize({150, 150});

  character_object_->AddComponent<CharacterMover>();

  character_object2_ = CreateGameObject("Character");
  character_object2_->SetParent(character_object_);
  character_object2_->GetComponent<TransformComponent>()->SetPosition({150, 150, 0});
  auto* char_sprite2 = character_object2_->AddComponent<SpriteRenderer>();
  char_sprite2->SetTexture(texture_character_.Get());
  char_sprite2->SetSize({150, 150});
  character_object2_->AddComponent<CharacterMover>();

  additive_ = CreateGameObject("Additive");
  additive_->GetComponent<TransformComponent>()->SetPosition({0, -3, -1.0f});
  auto* additive_sprite = additive_->AddComponent<SpriteRenderer>();
  additive_sprite->SetTexture(texture_additive_.Get());
  additive_sprite->SetSize({1.0f, 1.0f});
  additive_sprite->SetRenderLayer(RenderLayer::Transparent);
  additive_sprite->SetBillboardMode(Billboard::Mode::Spherical);
  additive_sprite->SetBlendMode(Rendering::BlendMode::Additive);
  additive_sprite->SetColor({0.0f, 1.0f, 0.0f, 0.5f});

  // Animated Background Sprite Example
  // Using texture_background_ (576x324) as a sprite sheet with mock animation frames
  // Assume 9 frames arranged in a 3x3 grid (each frame ~192x108 with padding)
  animated_bg_object_ = CreateGameObject("AnimatedBackground");
  // Position in screen space (UI pass uses screen coordinates)
  animated_bg_object_->GetComponent<TransformComponent>()->SetPosition({0.0f, -3.0f, 0.0f});

  auto* bg_renderer = animated_bg_object_->AddComponent<SpriteRenderer>();
  bg_renderer->SetTexture(texture_ao_.Get());
  bg_renderer->SetSize({15.0f, 15.0f});  // Full frame size
  bg_renderer->SetRenderLayer(RenderLayer::Transparent);
  bg_renderer->SetPivot(Pivot::Preset::Bottom);
  bg_renderer->SetDoubleSided(true);
  bg_renderer->SetBillboardMode(Billboard::Mode::None);
  bg_renderer->SetColor({1.0f, 0.0f, 0.0f, 0.5f});

  auto* bg_anim = animated_bg_object_->AddComponent<AnimatedSprite>();

  // Configure sprite sheet: treat the 576x324 texture as a 3x3 grid of animation frames
  SpriteSheet::FrameConfig bg_sheet_config;
  bg_sheet_config.sheet_size = {1024, 1024};
  bg_sheet_config.frame_size = {128, 128};
  bg_sheet_config.orientation = SpriteSheet::Orientation::Horizontal;
  bg_sheet_config.padding = {0, 0};
  bg_sheet_config.margin = {0, 0};

  bg_anim->SetSpriteSheetConfig(bg_sheet_config);
  bg_anim->SetFrameCount(64);          // 3x3 = 9 frames
  bg_anim->SetFramesPerSecond(30.0f);  // 10 FPS animation
  bg_anim->SetLoopEnabled(true);
  bg_anim->SetPlayOnStart(true);

  // Text
  {
    auto text_obj = CreateGameObject("Basic UI Text");

    // Position in screen space - center of screen (1920x1080 -> 960x540)
    text_obj->GetTransform()->SetPosition(DirectX::XMFLOAT3(960.0f, 540.0f, 0.0f));

    // Add TextRenderer Component
    auto* text = text_obj->AddComponent<TextRenderer>();

    // Configure text
    text->SetText(L"Hello, World!Hello, World!Hello, World!\nこんにちは！");
    text->SetFont(Font::FontFamily::ZenOldMincho);
    text->SetPixelSize(48.0f);
    text->SetColor({0.0f, 1.0f, 0.0f, 1.0f});
    text->SetHorizontalAlign(Text::HorizontalAlign::Left);
    text->SetVerticalAlign(Text::VerticalAlign::Center);
    text->SetPivot(Pivot::Preset::Center);

    // Set as UI element
    text->SetRenderLayer(RenderLayer::UI);
    text->SetLayerId(100);  // Higher layer = rendered later
    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "UI Text size: {} x {}", text->GetSize().x, text->GetSize().y);
  }

  // Text
  text_obj2_ = CreateGameObject("Basic Text 2");
  // text_obj2_->SetParent(cube_object_);
  // Position in screen space
  text_obj2_->GetTransform()->SetPosition(DirectX::XMFLOAT3(0.0f, -3.0f, 1.0f));

  // Add TextRenderer Component
  auto* text2 = text_obj2_->AddComponent<TextRenderer>();

  // Configure text
  text2->SetText(L"Hello, World!\n Bye");
  text2->SetFont(Font::FontFamily::ZenOldMincho);
  text2->SetPixelSize(1.0f);
  text2->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
  text2->SetRenderLayer(RenderLayer::Transparent);  // Use transparent for alpha blending
  text2->SetBillboardMode(Billboard::Mode::None);
  // text2->SetBlendMode(Rendering::BlendMode::AlphaBlend);

  text2->SetHorizontalAlign(Text::HorizontalAlign::Center);
  text2->SetVerticalAlign(Text::VerticalAlign::Center);
  text2->SetPivot(Pivot::Preset::Center);
  // text2->SetDepthTest(true);
  // text2->SetDepthWrite(true);
  text2->SetDoubleSided(true);
}

void TestScene::OnPostUpdate(float dt) {
  rotation_angle_ += rotation_speed_ * dt;

  cube_object_->GetComponent<TransformComponent>()->SetRotationEulerDegree({0.0f, rotation_angle_, 0.0f});
  cube_object2_->GetComponent<TransformComponent>()->SetRotationEulerDegree({0.0f, rotation_angle_, 0.0f});
}

void TestScene::OnRender(FramePacket& /* packet */) {
  auto* debug_drawer = GetContext()->GetDebugDrawer();
  if (!debug_drawer) return;

  // Draw grid
  DebugDrawer::GridConfig grid_config;
  grid_config.size = 20.0f;
  grid_config.cell_size = 1.0f;
  grid_config.y_level = 0.0f;
  grid_config.color = {0.5f, 0.5f, 0.5f, 1.0f};
  debug_drawer->DrawGrid(grid_config);

  // Draw axis gizmo at world origin
  DebugDrawer::AxisGizmoConfig axis_config;
  axis_config.position = {0.0f, 0.0f, 0.0f};
  axis_config.length = 2.0f;
  debug_drawer->DrawAxisGizmo(axis_config);

  // Optional: Draw bounding box around cube
  debug_drawer->DrawBox({-5, 2, -5}, {1, 1, 1}, {1, 0, 0, 1});

  debug_drawer->DrawSphere({-5, 2, -5}, 3, {1, 0, 0, 1}, 32);
}

void TestScene::OnExit() {
  character_object_ = nullptr;
  character_object2_ = nullptr;
  camera_object_ = nullptr;
  cube_object_ = nullptr;
  cube_object2_ = nullptr;
  terrain_plane_ = nullptr;
  text_obj_ = nullptr;
  text_obj2_ = nullptr;
  animated_bg_object_ = nullptr;
  additive_ = nullptr;
}

void TestScene::SetupCamera() {
  camera_object_ = CreateGameObject("MainCamera");
  auto* camera_transform = camera_object_->GetComponent<TransformComponent>();
  camera_transform->SetPosition({0.0f, 0.0f, -10.0f});  // Move camera back

  auto* camera = camera_object_->AddComponent<CameraComponent>();
  camera->SetPerspective(DirectX::XM_PIDIV4, 16.0f / 9.0f, 0.1f, 1000.0f);

  auto* controller = camera_object_->AddComponent<FreeCameraController>();
  controller->SetMovementSpeed(15.0f);
  controller->SetRotationSpeed(1.5f);
  controller->SetSmoothness(8.0f);

  SetActiveCamera(camera);
}
