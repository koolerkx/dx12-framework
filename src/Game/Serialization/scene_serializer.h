#pragma once

#include <filesystem>
#include <string>

class IScene;

class SceneSerializer {
 public:
  static bool SaveScene(const IScene& scene, const std::string& name);
  static bool DumpSettings(const IScene& scene, const std::string& name);
  static bool SaveAndDump(const IScene& scene, const std::string& name);

 private:
  static std::filesystem::path GetScenesDirectory();
};
