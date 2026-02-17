#pragma once

#include <cstdint>

#include "Asset/asset_manager.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/Renderer/particle_emitter.h"
#include "Component/behavior_component.h"
#include "Framework/Core/color.h"
#include "Graphic/Pipeline/pixel_shader_descriptors.h"
#include "ProceduralTexture/procedural_texture_factory.h"
#include "game_context.h"
#include "game_object.h"


class PlayerSpawnComponent : public BehaviorComponent<PlayerSpawnComponent> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void OnInit() override {
    auto& assets = GetContext()->GetAssetManager();
    const Mesh* mesh = CreateSpawnCubeMesh(assets);

    auto* renderer = GetOwner()->AddComponent<MeshRenderer>(MeshRenderer::Props{
      .mesh = mesh,
    });

    renderer->SetShaderWithParams<Graphics::NeonGridShader>(Graphics::NeonGridShader::Params{
      .grid_r = 0.0f,
      .grid_g = 1.0f,
      .grid_b = 0.8f,
      .grid_divisions = 4.0f,
      .fill_r = 0.0f,
      .fill_g = 0.9f,
      .fill_b = 0.7f,
      .fill_opacity = 0.5f,
      .grid_line_width = 0.05f,
      .glow_intensity = 1.1f,
    });

    RegisterProceduralTexture(assets);
    CreateParticleEmitter();
  }

  void OnFixedUpdate(float dt) override {
    auto transform = GetOwner()->GetTransform();
    if (transform) {
      constexpr float ROTATION_SPEED_DEGREES = 15.0f;
      auto rot_degrees = transform->GetRotationDegrees();
      transform->SetRotationEulerDegree(rot_degrees + (Math::Vector3::One * ROTATION_SPEED_DEGREES) * dt);
    }
  }

 private:
  static constexpr uint32_t PROCEDURAL_CIRCLE_SIZE = 64;
  static constexpr const char* PROCEDURAL_CIRCLE_KEY = "procedural:circle_128";

  static void RegisterProceduralTexture(AssetManager& assets) {
    auto pixels = GenerateProceduralTexture({.size = PROCEDURAL_CIRCLE_SIZE, .falloff = 10.0f, .shape = ProceduralShape::Circle});
    assets.CreateTextureFromPixels(PROCEDURAL_CIRCLE_KEY, pixels.data(), PROCEDURAL_CIRCLE_SIZE, PROCEDURAL_CIRCLE_SIZE);
  }

  void CreateParticleEmitter() {
    auto* emitter = GetOwner()->AddComponent<ParticleEmitter>(ParticleEmitter::Props{
      .texture_path = PROCEDURAL_CIRCLE_KEY,
      .max_particles = 200,
      .emit_rate = 25.0f,
      .particle_lifetime = 4.0f,
      .particle_size = {0.25f, 0.25f},
      .start_color = {0.3f, 0.6f, 1.0f, 0.8f},
      .end_color = {0.1f, 0.3f, 0.9f, 0.0f},
      .start_speed = 0.2f,
      .speed_variation = 0.15f,
      .gravity = {0.0f, 0.1f, 0.0f},
      .loop = true,
      .blend_mode = Rendering::BlendMode::Additive,
      .spawn_offset = {0.0f, -1.5f, 0.0f},
      .spawn_shape = SpawnShape::Disk,
      .spawn_radius = 1.25f,
      .fade_in_ratio = 0.15f,
      .fade_out_ratio = 0.3f,
      .emissive_intensity = 2.5f,
    });
    emitter->Play();
  }

  static const Mesh* CreateSpawnCubeMesh(AssetManager& assets) {
    using colors::ColorFromHex;
    return assets.CreateCube("player_spawn_cube",
      {{
        // Green
        // ColorFromHex("#00ffcc"),
        // ColorFromHex("#00fa9a"),
        // ColorFromHex("#00ced1"),
        // ColorFromHex("#20b2aa"),
        // ColorFromHex("#7fffd4"),
        // ColorFromHex("#00ff7f"),
        // ColorFromHex("#008b8b"),
        // ColorFromHex("#adff2f"),

        // Blue
        ColorFromHex("#00a2ff"),  // 0: Bright Azure
        ColorFromHex("#001eff"),  // 1: Electric Blue
        ColorFromHex("#00ffff"),  // 2: Cyan Glitch
        ColorFromHex("#0051ff"),  // 3: Royal Neon
        ColorFromHex("#b3e5ff"),  // 4: Crystal Blue
        ColorFromHex("#0072ff"),  // 5: Tech Blue
        ColorFromHex("#00008b"),  // 6: Deep Navy
        ColorFromHex("#1e90ff"),  // 7: Dodger Blue
      }});
  }
};
