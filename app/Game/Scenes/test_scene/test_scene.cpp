#include "test_scene.h"

#include <DirectXMath.h>

#include "Asset/asset_manager.h"
#include "Component/free_camera_controller.h"
#include "Component/mesh_renderer.h"
#include "Component/sprite_renderer.h"
#include "Component/text_renderer.h"
#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Frame/frame_packet.h"
#include "character_mover_component.h"
#include "test_scene.h"

static float rotation_angle_ = 0.0f;
static float rotation_speed_ = 15.0f;

void TestScene::OnEnter(AssetManager& asset_manager) {
  texture_background_ = asset_manager.LoadTexture("Content/textures/result_bg_1.png");
  texture_character_ = asset_manager.LoadTexture("Content/textures/ship_J.png");

  SetupCamera();

  // Create terrain plane
  terrain_plane_ = CreateGameObject("TerrainPlane");
  auto* plane_transform = terrain_plane_->GetComponent<TransformComponent>();
  plane_transform->SetPosition({0.0f, 0.0f, 0.0f});
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

  // Text
  {
    auto text_obj = CreateGameObject("Basic UI Text");

    // Position in screen space
    text_obj->GetTransform()->SetPosition(DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));

    // Add TextRenderer Component
    auto* text = text_obj->AddComponent<TextRenderer>();

    // Configure text
    text->SetText(L"Hello, World!\nこんにちは！ようこそ、世界へ。");
    text->SetFont(Font::FontFamily::ZenOldMincho);
    text->SetPixelSize(48.0f);
    text->SetColor({0.0f, 1.0f, 0.0f, 1.0f});
    text->SetHorizontalAlign(Text::HorizontalAlign::Right);
    text->SetVerticalAlign(Text::VerticalAlign::Center);

    // Set as UI element
    text->SetRenderPassTag(RenderPassTag::Ui);
    text->SetLayerId(100);  // Higher layer = rendered later
    std::cout << text->GetSize().x << " " << text->GetSize().y << std::endl;
  }

  // Text
  text_obj2_ = CreateGameObject("Basic Text 2");
  // text_obj2_->SetParent(cube_object_);
  // Position in screen space
  text_obj2_->GetTransform()->SetPosition(DirectX::XMFLOAT3(10.0f, 1.0f, 0.0f));

  // Add TextRenderer Component
  auto* text2 = text_obj2_->AddComponent<TextRenderer>();

  // Configure text
  text2->SetText(L"Hello, World!");
  text2->SetFont(Font::FontFamily::ZenOldMincho);
  text2->SetPixelSize(10.0f);
  text2->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
  text2->SetRenderPassTag(RenderPassTag::WorldTransparent);  // Use transparent for alpha blending
  text2->SetBillboardMode(Billboard::Mode::Cylindrical);
}

void TestScene::OnPostUpdate(float dt) {
  rotation_angle_ += rotation_speed_ * dt;

  cube_object_->GetComponent<TransformComponent>()->SetRotationEulerDegree({0.0f, rotation_angle_, 0.0f});
  cube_object2_->GetComponent<TransformComponent>()->SetRotationEulerDegree({0.0f, rotation_angle_, 0.0f});
}

void TestScene::OnRender(FramePacket& /*packet*/) {
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
