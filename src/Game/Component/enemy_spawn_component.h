#pragma once

#include <cstdint>

#include "Component/Renderer/mesh_renderer.h"
#include "Component/Renderer/particle_emitter.h"
#include "Component/behavior_component.h"
#include "Framework/Asset/asset_manager.h"
#include "Framework/Core/color.h"
#include "Framework/Shader/shader_descriptor.h"
#include "Framework/Shader/shader_registration.h"
#include "Framework/Shader/vertex_shaders.h"
#include "NeonGridCB.generated.h"
#include "ProceduralTexture/procedural_texture_factory.h"
#include "game_context.h"
#include "game_object.h"

class EnemySpawnComponent : public BehaviorComponent<EnemySpawnComponent> {
 public:
  using BehaviorComponent::BehaviorComponent;

  void OnInit() override {
    auto& assets = GetContext()->GetAssetManager();
    MeshHandle mesh = CreateSpawnCubeMesh(assets);

    if (auto* reg = GetContext()->GetShaderRegistration()) {
      ShaderDescriptor desc{
        .id = HashShaderName("NeonGrid"),
        .name = "NeonGrid",
        .vs_path = VS::Basic3D::PATH,
        .ps_path = L"Content/shaders/neon_grid.ps.cso",
        .vertex_format = VS::Basic3D::VERTEX_FORMAT,
        .default_settings =
          {
            .blend_mode = Rendering::BlendMode::Additive,
            .depth_test = true,
            .double_sided = true,
            .render_target_format = Rendering::RenderTargetFormat::HDR,
          },
      };
      reg->RegisterShader(desc);
    }

    auto* renderer = GetOwner()->AddComponent<MeshRenderer>(MeshRenderer::Props{
      .mesh_handle = mesh,
    });

    renderer->SetShaderId(HashShaderName("NeonGrid"));
    renderer->SetRenderLayer(RenderLayer::Transparent);
    renderer->SetRenderSettings({
      .blend_mode = Rendering::BlendMode::Additive,
      .depth_test = true,
      .double_sided = true,
      .render_target_format = Rendering::RenderTargetFormat::HDR,
    });
    NeonGridCB neon_grid_params{};
    neon_grid_params.gridColor = {1.0f, 0.0f, 0.0f};
    neon_grid_params.gridDivisions = 4.0f;
    neon_grid_params.fillColor = {1.0f, 0.2f, 0.1f};
    neon_grid_params.fillOpacity = 0.5f;
    neon_grid_params.gridLineWidth = 0.05f;
    neon_grid_params.glowIntensity = 1.1f;
    renderer->SetCustomData(neon_grid_params);

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
      .start_color = {1.0f, 0.3f, 0.15f, 0.8f},
      .end_color = {0.8f, 0.1f, 0.05f, 0.0f},
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

  static MeshHandle CreateSpawnCubeMesh(AssetManager& assets) {
    using colors::ColorFromHex;
    return assets.CreateCubeMesh("enemy_spawn_cube",
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

        ColorFromHex("#ff0000"),  // 0: Pure Red
        ColorFromHex("#ff4d4d"),  // 1: Light Coral
        ColorFromHex("#8b0000"),  // 2: Dark Blood
        ColorFromHex("#ff0055"),  // 3: Red Pink Neon
        ColorFromHex("#ffffff"),  // 4: Pure White
        ColorFromHex("#ff2400"),  // 5: Scarlet
        ColorFromHex("#4a0000"),  // 6: Deep Maroon
        ColorFromHex("#ff6600"),  // 7: Vivid Orange
      }});
  }
};
