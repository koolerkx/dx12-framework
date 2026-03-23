#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_set>

class IScene;
class AssetManager;
class GameObject;
class IComponentBase;

enum class LoadScope : uint8_t { Both, SceneOnly, SettingsOnly };

class SceneSerializer {
 public:
  using ComponentFactory = std::function<IComponentBase*(GameObject*)>;

  static void RegisterComponentFactory(const std::string& type_name, ComponentFactory factory);

  static bool SaveScene(const IScene& scene, const std::string& name, const std::unordered_set<GameObject*>& excluded = {});
  static bool DumpSettings(const IScene& scene, const std::string& name);
  static bool SaveAndDump(const IScene& scene, const std::string& name, const std::unordered_set<GameObject*>& excluded = {});

  static bool LoadScene(IScene& scene, const std::string& name);
  static bool LoadSettings(IScene& scene, const std::string& name);
  static bool Load(IScene& scene, const std::string& name, LoadScope scope);

  static std::filesystem::path GetScenesDirectory();
};
