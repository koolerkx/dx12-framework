#include "test_scene.h"

#include "Asset/asset_manager.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/Renderer/sprite_renderer.h"
#include "Component/Renderer/text_renderer.h"
#include "Component/Renderer/ui_sprite_renderer.h"
#include "Component/Renderer/ui_text_renderer.h"
#include "Component/camera_component.h"
#include "Component/pivot_type.h"
#include "Component/point_light_component.h"
#include "Component/sprite_sheet_helper.h"
#include "Component/transform_component.h"
#include "Debug/debug_drawer.h"
#include "Frame/frame_packet.h"
#include "Framework/Core/color.h"
#include "Framework/Input/input.h"
#include "Framework/Logging/logger.h"
#include "Framework/Math/Math.h"
#include "Scenes/test_scene/character_mover_component.h"
#include "Scripts/free_camera_controller.h"
#include "scene_id.h"
#include "scene_manager.h"
#include "test_scene.h"

using Math::Vector3;

static float rotation_angle_ = 0.0f;
static float rotation_speed_ = 15.0f;

void TestScene::OnEnter(AssetManager& asset_manager) {
  texture_background_ = asset_manager.LoadTexture("Content/textures/result_bg_1.png");
  texture_character_ = asset_manager.LoadTexture("Content/textures/ship_J.png");
  texture_ao_ = asset_manager.LoadTexture("Content/textures/metal_plate_ao_1k.png");
  texture_additive_ = asset_manager.LoadTexture("Content/textures/sun_additive.png");

  GetBackgroundSetting().SetSkybox("Content/skybox/sunflowers_puresky_standard_cubemap_4k.hdr", asset_manager);
  auto& light = GetLightSetting();
  light.SetAzimuth(45.0f);
  light.SetElevation(55.0f);

  SetupCamera();

  auto* terrain_plane = CreateGameObject("TerrainPlane", {.position = {0, -5, 0}, .scale = {20, 1, 20}});
  terrain_plane->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh = asset_manager.GetDefaultMesh(DefaultMesh::Plane),
    .texture = texture_background_.Get(),
    .color = colors::White,
  });

  cube_object_ = CreateGameObject("Cube", {.scale = {2, 2, 2}});
  cube_object_->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh = asset_manager.GetDefaultMesh(DefaultMesh::Cube),
    .texture = texture_background_.Get(),
    .color = colors::White,
  });

  cube_object2_ = CreateGameObject("Cube2", {.position = {1, 1, 1}});
  cube_object2_->SetParent(cube_object_);
  cube_object2_->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh = asset_manager.GetDefaultMesh(DefaultMesh::Cube),
    .texture = texture_background_.Get(),
    .color = colors::White,
  });

  auto* cube_object3 = CreateGameObject("PivotCube2", {.position = {0, -5.0f, -3}, .anchor = {0, -0.5f, 0}});
  cube_object3->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh = asset_manager.GetDefaultMesh(DefaultMesh::Cube),
    .texture = texture_background_.Get(),
    .color = colors::White,
  });

  auto* character_object = CreateGameObject("Character", {.position = {500, 500, 0}});
  character_object->AddComponent<UISpriteRenderer>(UISpriteRenderer::Props{
    .texture = texture_character_.Get(),
    .size = {150, 150},
  });
  character_object->AddComponent<CharacterMover>();

  auto* character_object2 = CreateGameObject("Character2", {.position = {150, 150, 0}});
  character_object2->SetParent(character_object);
  character_object2->AddComponent<UISpriteRenderer>(UISpriteRenderer::Props{
    .texture = texture_character_.Get(),
    .size = {150, 150},
  });
  character_object2->AddComponent<CharacterMover>();

  auto* additive = CreateGameObject("Additive", {.position = {0, -3, -1}});
  additive->AddComponent<SpriteRenderer>(SpriteRenderer::Props{
    .texture = texture_additive_.Get(),
    .color = colors::WithAlpha(colors::Lime, 0.5f),
    .size = {1, 1},
    .billboard_mode = Billboard::Mode::Spherical,
    .blend_mode = Rendering::BlendMode::Additive,
  });

  auto* animated_bg = CreateGameObject("AnimatedBackground", {.position = {0, -3, 0}});
  auto* bg_renderer = animated_bg->AddComponent<SpriteRenderer>(SpriteRenderer::Props{
    .texture = texture_ao_.Get(),
    .color = colors::WithAlpha(colors::Red, 0.5f),
    .size = {15, 15},
    .pivot = {0.5f, 0.5f},
    .double_sided = true,
  });

  sphere_object_ = CreateGameObject("Sphere", {.position = {-4, 0, 0}});
  sphere_object_->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh = asset_manager.GetDefaultMesh(DefaultMesh::Sphere),
    .texture = texture_background_.Get(),
    .color = colors::White,
  });

  pivot_cube_ = CreateGameObject("PivotCube", {.position = {5, 0, 5}, .pivot = {0.5f, 0, 0}});
  pivot_cube_->AddComponent<MeshRenderer>(MeshRenderer::Props{
    .mesh = asset_manager.GetDefaultMesh(DefaultMesh::Cube),
    .texture = texture_background_.Get(),
    .color = colors::White,
  });

  SpriteSheet::FrameConfig bg_sheet_config;
  bg_sheet_config.sheet_size = {1024, 1024};
  bg_sheet_config.frame_size = {128, 128};
  bg_sheet_config.orientation = SpriteSheet::Orientation::Horizontal;
  bg_sheet_config.padding = {0, 0};
  bg_sheet_config.margin = {0, 0};

  auto& animator = bg_renderer->GetAnimator();
  animator.SetSpriteSheetConfig(bg_sheet_config);
  animator.SetFrameCount(64);
  animator.SetFramesPerSecond(30.0f);
  animator.SetLoopEnabled(true);
  animator.Play();

  {
    auto* text_obj = CreateGameObject("Basic UI Text", {.position = {960, 540, 0}});
    auto* text = text_obj->AddComponent<UITextRenderer>(UITextRenderer::Props{
      .text = L"Hello, World!Hello, World!Hello, World!\nこんにちは！",
      .pixel_size = 48.0f,
      .color = colors::Lime,
      .v_align = Text::VerticalAlign::Center,
      .pivot = {0.5f, 0.5f},
      .layer_id = 100,
    });
    Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "UI Text size: {} x {}", text->GetSize().x, text->GetSize().y);
  }

  auto* point_light1 = CreateGameObject("PointLight_Red", {.position = {3, 1, 0}});
  point_light1->AddComponent<PointLightComponent>(PointLightComponent::Props{
    .color = {1.0f, 0.2f, 0.2f},
    .intensity = 2.0f,
    .radius = 15.0f,
    .falloff = 1.0f,
  });

  auto* point_light2 = CreateGameObject("PointLight_Blue", {.position = {-3, 1, 0}});
  point_light2->AddComponent<PointLightComponent>(PointLightComponent::Props{
    .color = {0.2f, 0.4f, 1.0f},
    .intensity = 2.0f,
    .radius = 15.0f,
    .falloff = 1.0f,
  });

  auto* text_obj2 = CreateGameObject("Basic Text 2", {.position = {0, -3, 1}});
  text_obj2->AddComponent<TextRenderer>(TextRenderer::Props{
    .text = L"Hello, World!\n Bye",
    .pixel_size = 1.0f,
    .color = colors::White,
    .h_align = Text::HorizontalAlign::Center,
    .v_align = Text::VerticalAlign::Center,
    .billboard_mode = Billboard::Mode::Cylindrical,
    .pivot = {0.5f, 0.5f},
    .double_sided = true,
  });
}

void TestScene::OnPostUpdate(float dt) {
  rotation_angle_ += rotation_speed_ * dt;

  cube_object_->GetComponent<TransformComponent>()->SetRotationEulerDegree({0.0f, rotation_angle_, 0.0f});
  cube_object2_->GetComponent<TransformComponent>()->SetRotationEulerDegree({0.0f, rotation_angle_, 0.0f});
  pivot_cube_->GetComponent<TransformComponent>()->SetRotationEulerDegree({0.0f, rotation_angle_, 0.0f});

  auto* input = GetContext()->GetInput();
  if (input && input->GetKeyDown(Keyboard::KeyCode::F1)) {
    GetContext()->GetSceneManager()->RequestLoad(SceneId::CUBE_SCENE);
  }
}

void TestScene::OnDebugDraw(DebugDrawer& drawer) {
  DebugDrawer::GridConfig grid_config;
  grid_config.size = 20.0f;
  grid_config.cell_size = 1.0f;
  grid_config.y_level = 0.0f;
  grid_config.color = colors::Gray;
  drawer.DrawGrid(grid_config);

  DebugDrawer::AxisGizmoConfig axis_config;
  axis_config.position = Vector3::Zero;
  axis_config.length = 2.0f;
  drawer.DrawAxisGizmo(axis_config);

  drawer.DrawWireCube({-6, 1, -6}, {-4, 3, -4}, colors::Red);

  drawer.DrawWireSphere({-5, 2, -5}, 3, colors::Red, 32);

  DebugDrawer::AxisGizmoConfig pivot_axis;
  pivot_axis.position = {5.0f, 0.0f, 5.0f};
  pivot_axis.length = 1.0f;
  drawer.DrawAxisGizmo(pivot_axis);

  auto pivot_world = pivot_cube_->GetComponent<TransformComponent>()->GetWorldMatrix().TransformPoint({0.5f, 0.0f, 0.0f});
  drawer.DrawWireSphere(pivot_world, 0.1f, colors::Yellow, 16);
}

void TestScene::OnExit() {
  cube_object_ = nullptr;
  cube_object2_ = nullptr;
  pivot_cube_ = nullptr;
  sphere_object_ = nullptr;
}

void TestScene::SetupCamera() {
  auto* camera_obj = CreateGameObject("MainCamera", {.position = {0, 0, -10}});
  auto* camera = camera_obj->AddComponent<CameraComponent>();
  camera_obj->AddComponent<FreeCameraController>(FreeCameraController::Props{
    .movement_speed = 15.0f,
    .rotation_speed = 1.5f,
    .smoothness = 8.0f,
  });
  GetCameraSetting().Register(camera);
}
