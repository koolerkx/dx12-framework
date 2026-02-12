#include "scene_serializer.h"

#include <functional>
#include <string>
#include <unordered_map>

#include "Framework/Logging/logger.h"
#include "Framework/Serialize/serialize_document.h"
#include "Framework/Serialize/serialize_node.h"
#include "Framework/UUID/uuid.h"
#include "Framework/Utils/path_utils.h"
#include "Game/Component/Renderer/mesh_renderer.h"
#include "Game/Component/Renderer/particle_emitter.h"
#include "Game/Component/Renderer/sprite_renderer.h"
#include "Game/Component/Renderer/text_renderer.h"
#include "Game/Component/Renderer/ui_sprite_renderer.h"
#include "Game/Component/Renderer/ui_text_renderer.h"
#include "Game/Component/camera_component.h"
#include "Game/Component/component.h"
#include "Game/Component/model_component.h"
#include "Game/Component/point_light_component.h"
#include "Game/Scenes/test_scene/character_mover_component.h"
#include "Game/Scripts/free_camera_controller.h"
#include "Game/game_object.h"
#include "Game/scene.h"

namespace {

using ComponentFactory = std::function<IComponentBase*(GameObject*)>;

const std::unordered_map<std::string, ComponentFactory>& GetComponentFactories() {
  static const std::unordered_map<std::string, ComponentFactory> factories = {
    {"CameraComponent", [](GameObject* o) { return o->AddComponent<CameraComponent>(); }},
    {"SpriteRenderer", [](GameObject* o) { return o->AddComponent<SpriteRenderer>(); }},
    {"MeshRenderer", [](GameObject* o) { return o->AddComponent<MeshRenderer>(); }},
    {"TextRenderer", [](GameObject* o) { return o->AddComponent<TextRenderer>(); }},
    {"UITextRenderer", [](GameObject* o) { return o->AddComponent<UITextRenderer>(); }},
    {"UISpriteRenderer", [](GameObject* o) { return o->AddComponent<UISpriteRenderer>(); }},
    {"ParticleEmitter", [](GameObject* o) { return o->AddComponent<ParticleEmitter>(); }},
    {"PointLightComponent", [](GameObject* o) { return o->AddComponent<PointLightComponent>(); }},
    {"ModelComponent", [](GameObject* o) { return o->AddComponent<ModelComponent>(); }},
    {"FreeCameraController", [](GameObject* o) { return o->AddComponent<FreeCameraController>(); }},
    {"CharacterMover", [](GameObject* o) { return o->AddComponent<CharacterMover>(); }},
  };
  return factories;
}

ShadowAlgorithm ParseShadowAlgorithm(const std::string& name) {
  if (name == "Hard") return ShadowAlgorithm::Hard;
  if (name == "PCF3x3") return ShadowAlgorithm::PCF3x3;
  if (name == "PoissonDisk") return ShadowAlgorithm::PoissonDisk;
  if (name == "RotatedPoissonDisk") return ShadowAlgorithm::RotatedPoissonDisk;
  if (name == "PCSS") return ShadowAlgorithm::PCSS;
  return ShadowAlgorithm::PCF3x3;
}

bool IsExcluded(const GameObject* go, const std::unordered_set<GameObject*>& excluded) {
  for (auto* current = go; current; current = current->GetParent())
    if (excluded.contains(const_cast<GameObject*>(current))) return true;
  return false;
}

}  // namespace

bool SceneSerializer::SaveScene(const IScene& scene, const std::string& name,
                                const std::unordered_set<GameObject*>& excluded) {
  framework::SerializeDocument doc;
  auto& root = doc.Root();
  root.Write("SceneName", name);

  auto entities = root.BeginSequence("Entities");

  for (const auto& go : scene.GetGameObjects()) {
    if (!go || go->IsTransient()) continue;
    if (!excluded.empty() && IsExcluded(go.get(), excluded)) continue;

    auto entity = entities.AddSequenceElement();
    entity.Write("UUID", go->GetUUID().ToString());
    entity.Write("Name", go->GetName());

    if (!go->IsActive()) {
      entity.Write("Active", false);
    }

    if (go->GetParent()) {
      entity.Write("ParentUUID", go->GetParent()->GetUUID().ToString());
    }

    auto components = entity.BeginSequence("Components");

    for (const auto& comp : go->GetComponents()) {
      if (!comp) continue;

      auto comp_node = components.AddSequenceElement();
      comp_node.Write("Type", std::string(comp->GetTypeName()));

      auto data = comp_node.BeginMap("Data");
      comp->OnSerialize(data);
    }
  }

  auto scenes_dir = GetScenesDirectory();
  auto path = scenes_dir / (name + ".scene.yaml");
  return doc.SaveToFile(path);
}

bool SceneSerializer::DumpSettings(const IScene& scene, const std::string& name) {
  framework::SerializeDocument doc;
  auto& root = doc.Root();

  {
    auto bg = root.BeginMap("Background");
    const auto& bg_setting = scene.GetBackgroundSetting();
    bg.Write("Mode", bg_setting.GetMode() == BackgroundMode::ClearColor ? "ClearColor" : "Skybox");
    auto color = bg_setting.GetClearColor();
    bg.WriteVec4("ClearColor", color.x, color.y, color.z, color.w);
    if (!bg_setting.GetSkyboxPath().empty()) {
      bg.Write("SkyboxPath", bg_setting.GetSkyboxPath());
    }
  }

  {
    auto light = root.BeginMap("Light");
    const auto& light_setting = scene.GetLightSetting();
    light.Write("Azimuth", light_setting.GetAzimuth());
    light.Write("Elevation", light_setting.GetElevation());
    light.Write("Intensity", light_setting.GetIntensity());
    auto dir_color = light_setting.GetDirectionalColor();
    light.WriteVec3("DirectionalColor", dir_color.x, dir_color.y, dir_color.z);
    light.Write("AmbientIntensity", light_setting.GetAmbientIntensity());
    auto amb_color = light_setting.GetAmbientColor();
    light.WriteVec3("AmbientColor", amb_color.x, amb_color.y, amb_color.z);
  }

  {
    auto shadow = root.BeginMap("Shadow");
    const auto& shadow_setting = scene.GetShadowSetting();
    shadow.Write("Enabled", shadow_setting.IsEnabled());
    shadow.Write("Resolution", shadow_setting.GetResolution());
    shadow.Write("CascadeCount", shadow_setting.GetCascadeCount());

    static const char* ALGORITHM_NAMES[] = {"Hard", "PCF3x3", "PoissonDisk", "RotatedPoissonDisk", "PCSS"};
    auto algo_index = static_cast<int>(shadow_setting.GetAlgorithm());
    if (algo_index < 0 || algo_index >= static_cast<int>(std::size(ALGORITHM_NAMES))) algo_index = 0;
    shadow.Write("Algorithm", ALGORITHM_NAMES[algo_index]);

    shadow.Write("ShadowDistance", shadow_setting.GetShadowDistance());
    shadow.Write("LightDistance", shadow_setting.GetLightDistance());
    shadow.Write("CascadeBlendRange", shadow_setting.GetCascadeBlendRange());
    shadow.Write("LightSize", shadow_setting.GetLightSize());

    auto shadow_color = shadow_setting.GetShadowColor();
    shadow.WriteVec3("ShadowColor", shadow_color.x, shadow_color.y, shadow_color.z);

    for (uint32_t i = 0; i < shadow_setting.GetCascadeCount(); ++i) {
      auto cascade = shadow.BeginMap("Cascade" + std::to_string(i));
      cascade.Write("DepthBias", shadow_setting.GetCascadeDepthBias(i));
      cascade.Write("NormalBias", shadow_setting.GetCascadeNormalBias(i));
    }
  }

  auto scenes_dir = GetScenesDirectory();
  auto path = scenes_dir / (name + ".settings.yaml");
  return doc.SaveToFile(path);
}

bool SceneSerializer::SaveAndDump(const IScene& scene, const std::string& name,
                                  const std::unordered_set<GameObject*>& excluded) {
  bool scene_ok = SaveScene(scene, name, excluded);
  bool settings_ok = DumpSettings(scene, name);
  return scene_ok && settings_ok;
}

bool SceneSerializer::LoadScene(IScene& scene, const std::string& name) {
  auto path = GetScenesDirectory() / (name + ".scene.yaml");

  framework::SerializeDocument doc;
  if (!doc.LoadFromFile(path)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "Failed to load scene file: {}", path.string());
    return false;
  }

  auto& root = doc.Root();
  scene.SetSceneName(root.ReadString("SceneName", name));

  size_t entity_count = root.GetSequenceSize("Entities");
  Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "LoadScene '{}': {} entities", name, entity_count);

  // Pass 1: Create all GameObjects with UUIDs
  for (size_t i = 0; i < entity_count; ++i) {
    auto entities_seq = root.GetSequence("Entities");
    auto entity_node = entities_seq.GetSequenceElement(i);
    auto obj_name = entity_node.ReadString("Name", "GameObject");
    auto uuid_str = entity_node.ReadString("UUID");

    auto* go = scene.CreateGameObject(obj_name, {});
    go->SetActive(entity_node.ReadBool("Active", true));
    if (!uuid_str.empty()) {
      go->SetUUID(framework::UUID::FromString(uuid_str));
    }
  }

  // Pass 2: Restore hierarchy
  for (size_t i = 0; i < entity_count; ++i) {
    auto entities_seq = root.GetSequence("Entities");
    auto entity_node = entities_seq.GetSequenceElement(i);
    auto parent_uuid_str = entity_node.ReadString("ParentUUID");
    if (parent_uuid_str.empty()) continue;

    auto uuid_str = entity_node.ReadString("UUID");
    if (uuid_str.empty()) continue;

    auto* child = scene.FindGameObjectByUUID(framework::UUID::FromString(uuid_str));
    auto* parent = scene.FindGameObjectByUUID(framework::UUID::FromString(parent_uuid_str));
    if (child && parent) {
      child->SetParent(parent);
    }
  }

  // Pass 3: Create and deserialize components
  const auto& factories = GetComponentFactories();

  for (size_t i = 0; i < entity_count; ++i) {
    auto entities_seq = root.GetSequence("Entities");
    auto entity_node = entities_seq.GetSequenceElement(i);
    auto uuid_str = entity_node.ReadString("UUID");
    if (uuid_str.empty()) continue;
    auto* go = scene.FindGameObjectByUUID(framework::UUID::FromString(uuid_str));
    if (!go) {
      Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "  Entity not found by UUID: {}", uuid_str);
      continue;
    }

    auto components_seq = entity_node.GetSequence("Components");
    size_t comp_count = components_seq.GetSequenceSize();

    for (size_t j = 0; j < comp_count; ++j) {
      auto comp_node = components_seq.GetSequenceElement(j);
      auto type_name = comp_node.ReadString("Type");
      auto data_node = comp_node.GetMap("Data");

      if (type_name == "TransformComponent") {
        auto* transform = go->GetTransform();
        if (transform) transform->OnDeserialize(data_node);
        continue;
      }

      auto it = factories.find(type_name);
      if (it == factories.end()) {
        Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "    Unknown component type '{}', skipping", type_name);
        continue;
      }

      auto* component = it->second(go);
      if (component) {
        component->OnDeserialize(data_node);

        if (type_name == "CameraComponent") {
          auto* camera = static_cast<CameraComponent*>(component);
          scene.GetCameraSetting().Register(camera, camera->GetPriority());
          Logger::LogFormat(LogLevel::Info, LogCategory::Game, Logger::Here(), "    Registered camera from '{}'", go->GetName());
        }
      }
    }
  }

  Logger::LogFormat(
    LogLevel::Info, LogCategory::Game, Logger::Here(), "LoadScene '{}' complete, {} objects", name, scene.GetGameObjects().size());
  return true;
}

bool SceneSerializer::LoadSettings(IScene& scene, const std::string& name) {
  auto path = GetScenesDirectory() / (name + ".settings.yaml");

  framework::SerializeDocument doc;
  if (!doc.LoadFromFile(path)) {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Game, Logger::Here(), "Settings file not found: {}", path.string());
    return false;
  }

  const auto& root = doc.Root();

  // Background
  if (root.HasKey("Background")) {
    auto bg_node = root.GetMap("Background");
    auto& bg = scene.GetBackgroundSetting();
    Color clear_color = bg.GetClearColor();
    bg_node.ReadVec4("ClearColor", clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    bg.SetClearColorValue(clear_color);

    auto mode_str = bg_node.ReadString("Mode", "ClearColor");
    if (mode_str == "Skybox") {
      auto skybox_path = bg_node.ReadString("SkyboxPath");
      if (!skybox_path.empty() && scene.GetContext()) {
        bg.SetSkybox(skybox_path, scene.GetContext()->GetAssetManager());
      }
    } else {
      bg.SetMode(BackgroundMode::ClearColor);
    }
  }

  // Light
  if (root.HasKey("Light")) {
    auto light_node = root.GetMap("Light");
    auto& light = scene.GetLightSetting();
    light.SetAzimuth(light_node.ReadFloat("Azimuth", light.GetAzimuth()));
    light.SetElevation(light_node.ReadFloat("Elevation", light.GetElevation()));
    light.SetIntensity(light_node.ReadFloat("Intensity", light.GetIntensity()));
    Math::Vector3 dir_color = light.GetDirectionalColor();
    light_node.ReadVec3("DirectionalColor", dir_color.x, dir_color.y, dir_color.z);
    light.SetDirectionalColor(dir_color);
    light.SetAmbientIntensity(light_node.ReadFloat("AmbientIntensity", light.GetAmbientIntensity()));
    Math::Vector3 amb_color = light.GetAmbientColor();
    light_node.ReadVec3("AmbientColor", amb_color.x, amb_color.y, amb_color.z);
    light.SetAmbientColor(amb_color);
  }

  // Shadow
  if (root.HasKey("Shadow")) {
    auto shadow_node = root.GetMap("Shadow");
    auto& shadow = scene.GetShadowSetting();
    shadow.SetEnabled(shadow_node.ReadBool("Enabled", shadow.IsEnabled()));
    shadow.SetResolution(shadow_node.ReadUint("Resolution", shadow.GetResolution()));
    shadow.SetCascadeCount(shadow_node.ReadUint("CascadeCount", shadow.GetCascadeCount()));
    shadow.SetAlgorithm(ParseShadowAlgorithm(shadow_node.ReadString("Algorithm", "PCF3x3")));
    shadow.SetShadowDistance(shadow_node.ReadFloat("ShadowDistance", shadow.GetShadowDistance()));
    shadow.SetLightDistance(shadow_node.ReadFloat("LightDistance", shadow.GetLightDistance()));
    shadow.SetCascadeBlendRange(shadow_node.ReadFloat("CascadeBlendRange", shadow.GetCascadeBlendRange()));
    shadow.SetLightSize(shadow_node.ReadFloat("LightSize", shadow.GetLightSize()));
    Math::Vector3 shadow_color = shadow.GetShadowColor();
    shadow_node.ReadVec3("ShadowColor", shadow_color.x, shadow_color.y, shadow_color.z);
    shadow.SetShadowColor(shadow_color);

    for (uint32_t i = 0; i < shadow.GetCascadeCount(); ++i) {
      auto cascade_key = "Cascade" + std::to_string(i);
      if (shadow_node.HasKey(cascade_key)) {
        auto cascade_node = shadow_node.GetMap(cascade_key);
        shadow.SetCascadeDepthBias(i, cascade_node.ReadFloat("DepthBias", shadow.GetCascadeDepthBias(i)));
        shadow.SetCascadeNormalBias(i, cascade_node.ReadFloat("NormalBias", shadow.GetCascadeNormalBias(i)));
      }
    }
  }

  return true;
}

bool SceneSerializer::Load(IScene& scene, const std::string& name, LoadScope scope) {
  switch (scope) {
    case LoadScope::Both: {
      bool scene_ok = LoadScene(scene, name);
      bool settings_ok = LoadSettings(scene, name);
      return scene_ok && settings_ok;
    }
    case LoadScope::SceneOnly:
      return LoadScene(scene, name);
    case LoadScope::SettingsOnly:
      return LoadSettings(scene, name);
  }
  return false;
}

std::filesystem::path SceneSerializer::GetScenesDirectory() {
  auto project_root = FindProjectRoot();
  if (project_root) {
    return *project_root / "Content" / "scenes";
  }
  return std::filesystem::current_path() / "Content" / "scenes";
}
