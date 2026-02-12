#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

class IScene;
class AssetManager;

enum class LoadScope : uint8_t { Both, SceneOnly, SettingsOnly };

class SceneSerializer {
 public:
  static bool SaveScene(const IScene& scene, const std::string& name);
  static bool DumpSettings(const IScene& scene, const std::string& name);
  static bool SaveAndDump(const IScene& scene, const std::string& name);

  static bool LoadScene(IScene& scene, const std::string& name);
  static bool LoadSettings(IScene& scene, const std::string& name);
  static bool Load(IScene& scene, const std::string& name, LoadScope scope);

  static std::filesystem::path GetScenesDirectory();
};
