#pragma once

#include <cstdint>
#include <string>

#include "Asset/asset_manager.h"
#include "ProceduralTexture/procedural_texture_factory.h"
#include "Component/Renderer/mesh_renderer.h"
#include "Component/Renderer/particle_emitter.h"
#include "Component/behavior_component.h"
#include "Framework/Core/color.h"
#include "Graphic/Pipeline/pixel_shader_descriptors.h"
#include "game_context.h"
#include "game_object.h"

enum class SpawnType : uint8_t { Player, Enemy };

class SpawnPointComponent : public BehaviorComponent<SpawnPointComponent> {
 public:
  struct Props {
    SpawnType type = SpawnType::Enemy;
  };

  SpawnPointComponent(GameObject* owner, const Props& props) : BehaviorComponent(owner), type_(props.type) {
  }

  void OnInit() override {
    auto& assets = GetContext()->GetAssetManager();
    const Mesh* mesh = CreateMeshForType(assets, type_);

    auto* renderer = GetOwner()->AddComponent<MeshRenderer>(MeshRenderer::Props{
      .mesh = mesh,
    });

    auto shader_params = BuildShaderParams(type_);
    renderer->SetShaderWithParams<Graphics::NeonGridShader>(shader_params);

    RegisterProceduralTexture(assets);
    CreateParticleEmitter();
  }

  SpawnType GetSpawnType() const {
    return type_;
  }

  void OnFixedUpdate(float dt) override {
    auto transform = GetOwner()->GetTransform();
    if (transform) {
      constexpr float rotation_speed_degrees = 15.0f;

      auto rot_degrees = transform->GetRotationDegrees();
      transform->SetRotationEulerDegree(rot_degrees + (Math::Vector3::One * rotation_speed_degrees) * dt);
    }
  }

 private:
  SpawnType type_;

  static constexpr uint32_t PROCEDURAL_CIRCLE_SIZE = 64;
  static constexpr const char* PROCEDURAL_CIRCLE_KEY = "procedural:circle_128";

  static void RegisterProceduralTexture(AssetManager& assets) {
    auto pixels = GenerateProceduralTexture({.size = PROCEDURAL_CIRCLE_SIZE, .falloff = 10.0f, .shape = ProceduralShape::Circle});
    assets.CreateTextureFromPixels(PROCEDURAL_CIRCLE_KEY, pixels.data(), PROCEDURAL_CIRCLE_SIZE, PROCEDURAL_CIRCLE_SIZE);
  }

  void CreateParticleEmitter() {
    bool is_player = (type_ == SpawnType::Player);

    Vector4 start_color = is_player ? Vector4{0.3f, 0.6f, 1.0f, 0.8f} : Vector4{1.0f, 0.3f, 0.15f, 0.8f};
    Vector4 end_color = is_player ? Vector4{0.1f, 0.3f, 0.9f, 0.0f} : Vector4{0.8f, 0.1f, 0.05f, 0.0f};

    auto* emitter = GetOwner()->AddComponent<ParticleEmitter>(ParticleEmitter::Props{
      .texture_path = PROCEDURAL_CIRCLE_KEY,
      .max_particles = 200,
      .emit_rate = 25.0f,
      .particle_lifetime = 4.0f,
      .particle_size = {0.25f, 0.25f},
      .start_color = start_color,
      .end_color = end_color,
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

  static const Mesh* CreateMeshForType(AssetManager& assets, SpawnType type) {
    using colors::ColorFromHex;

    if (type == SpawnType::Player) {
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

    return assets.CreateCube("enemy_spawn_cube",
      {{
        // Red
        // ColorFromHex("#ff0000"),
        // ColorFromHex("#ff4500"),
        // ColorFromHex("#dc143c"),
        // ColorFromHex("#ff8c00"),
        // ColorFromHex("#ff2400"),
        // ColorFromHex("#b22222"),
        // ColorFromHex("#8b0000"),
        // ColorFromHex("#ff6347"),

        ColorFromHex("#ff0000"),  // 0: Pure Red (
        ColorFromHex("#ff4d4d"),  // 1: Light Coral
        ColorFromHex("#8b0000"),  // 2: Dark Blood
        ColorFromHex("#ff0055"),  // 3: Red Pink Neon
        ColorFromHex("#ffffff"),  // 4: Pure White
        ColorFromHex("#ff2400"),  // 5: Scarlet
        ColorFromHex("#4a0000"),  // 6: Deep Maroon
        ColorFromHex("#ff6600"),  // 7: Vivid Orange
      }});
  }

  static Graphics::NeonGridShader::Params BuildShaderParams(SpawnType type) {
    if (type == SpawnType::Player) {
      return {
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
      };
    }

    return {
      .grid_r = 1.0f,
      .grid_g = 0.0f,
      .grid_b = 0.0f,
      .grid_divisions = 4.0f,
      .fill_r = 1.0f,
      .fill_g = 0.2f,
      .fill_b = 0.1f,
      .fill_opacity = 0.5f,
      .grid_line_width = 0.05f,
      .glow_intensity = 1.1f,
    };
  }
};
