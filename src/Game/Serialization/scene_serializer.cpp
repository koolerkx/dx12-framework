#include "scene_serializer.h"

#include "Framework/Serialize/serialize_document.h"
#include "Framework/Serialize/serialize_node.h"
#include "Framework/Utils/path_utils.h"
#include "Game/Component/component.h"
#include "Game/game_object.h"
#include "Game/scene.h"

bool SceneSerializer::SaveScene(const IScene& scene, const std::string& name) {
  framework::SerializeDocument doc;
  auto& root = doc.Root();
  root.Write("SceneName", name);

  auto entities = root.BeginSequence("Entities");

  for (const auto& go : scene.GetGameObjects()) {
    if (!go) continue;

    auto entity = entities.AddSequenceElement();
    entity.Write("UUID", go->GetUUID().ToString());
    entity.Write("Name", go->GetName());

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

bool SceneSerializer::SaveAndDump(const IScene& scene, const std::string& name) {
  bool scene_ok = SaveScene(scene, name);
  bool settings_ok = DumpSettings(scene, name);
  return scene_ok && settings_ok;
}

std::filesystem::path SceneSerializer::GetScenesDirectory() {
  auto project_root = FindProjectRoot();
  if (project_root) {
    return *project_root / "Content" / "scenes";
  }
  return std::filesystem::current_path() / "Content" / "scenes";
}
