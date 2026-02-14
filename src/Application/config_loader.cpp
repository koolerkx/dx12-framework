#include "config_loader.h"

#include "Framework/Logging/logger.h"
#include "Framework/Serialize/serialize_document.h"
#include "Framework/Serialize/serialize_node.h"

namespace {

ShadowAlgorithm ParseShadowAlgorithm(const std::string& name) {
  if (name == "Hard") return ShadowAlgorithm::Hard;
  if (name == "PCF3x3") return ShadowAlgorithm::PCF3x3;
  if (name == "PoissonDisk") return ShadowAlgorithm::PoissonDisk;
  if (name == "RotatedPoissonDisk") return ShadowAlgorithm::RotatedPoissonDisk;
  if (name == "PCSS") return ShadowAlgorithm::PCSS;
  return ShadowAlgorithm::PCF3x3;
}

BackgroundMode ParseBackgroundMode(const std::string& name) {
  if (name == "Skybox") return BackgroundMode::Skybox;
  return BackgroundMode::ClearColor;
}

void LoadBloomConfig(const framework::SerializeNode& node, BloomConfig& bloom) {
  bloom.enabled = node.ReadBool("Enabled", bloom.enabled);
  bloom.mip_levels = node.ReadUint("MipLevels", bloom.mip_levels);
  bloom.threshold = node.ReadFloat("Threshold", bloom.threshold);
  bloom.intensity = node.ReadFloat("Intensity", bloom.intensity);
}

void LoadSSAOConfig(const framework::SerializeNode& node, SSAOConfig& ssao) {
  ssao.enabled = node.ReadBool("Enabled", ssao.enabled);
  ssao.radius = node.ReadFloat("Radius", ssao.radius);
  ssao.bias = node.ReadFloat("Bias", ssao.bias);
  ssao.intensity = node.ReadFloat("Intensity", ssao.intensity);
  ssao.sample_count = node.ReadUint("SampleCount", ssao.sample_count);
}

void LoadSMAAConfig(const framework::SerializeNode& node, SMAAConfig& smaa) {
  smaa.enabled = node.ReadBool("Enabled", smaa.enabled);
}

void LoadSceneDefaults(const framework::SerializeNode& root, SceneDefaults& defaults) {
  if (root.HasKey("Light")) {
    auto light = root.GetMap("Light");
    defaults.light_azimuth = light.ReadFloat("Azimuth", defaults.light_azimuth);
    defaults.light_elevation = light.ReadFloat("Elevation", defaults.light_elevation);
    defaults.light_intensity = light.ReadFloat("Intensity", defaults.light_intensity);
    light.ReadVec3("Color", defaults.light_color[0], defaults.light_color[1], defaults.light_color[2]);
    defaults.ambient_intensity = light.ReadFloat("AmbientIntensity", defaults.ambient_intensity);
    light.ReadVec3("AmbientColor", defaults.ambient_color[0], defaults.ambient_color[1], defaults.ambient_color[2]);
  }

  if (root.HasKey("Shadow")) {
    auto shadow = root.GetMap("Shadow");
    defaults.shadow_enabled = shadow.ReadBool("Enabled", defaults.shadow_enabled);
    defaults.shadow_resolution = shadow.ReadUint("Resolution", defaults.shadow_resolution);
    defaults.shadow_cascade_count = shadow.ReadUint("CascadeCount", defaults.shadow_cascade_count);
    defaults.shadow_algorithm = ParseShadowAlgorithm(shadow.ReadString("Algorithm", "PCF3x3"));
    defaults.shadow_distance = shadow.ReadFloat("ShadowDistance", defaults.shadow_distance);
    defaults.light_distance = shadow.ReadFloat("LightDistance", defaults.light_distance);
    defaults.cascade_blend_range = shadow.ReadFloat("CascadeBlendRange", defaults.cascade_blend_range);
    shadow.ReadVec3("ShadowColor", defaults.shadow_color[0], defaults.shadow_color[1], defaults.shadow_color[2]);
    defaults.light_size = shadow.ReadFloat("LightSize", defaults.light_size);
  }

  if (root.HasKey("Background")) {
    auto bg = root.GetMap("Background");
    defaults.background_mode = ParseBackgroundMode(bg.ReadString("Mode", "ClearColor"));
    bg.ReadVec4("ClearColor", defaults.clear_color[0], defaults.clear_color[1], defaults.clear_color[2], defaults.clear_color[3]);
  }
}

}  // namespace

AppConfig ConfigLoader::LoadFromFile(const std::filesystem::path& path) {
  AppConfig config;

  framework::SerializeDocument doc;
  if (!doc.LoadFromFile(path)) {
    Logger::LogFormat(LogLevel::Warn, LogCategory::Core, Logger::Here(), "Config file not found: {}, using defaults", path.string());
    return config;
  }

  const auto& root = doc.Root();

  config.window_width = root.ReadUint("WindowWidth", config.window_width);
  config.window_height = root.ReadUint("WindowHeight", config.window_height);
  config.vsync = root.ReadBool("VSync", config.vsync);

  if (root.HasKey("Bloom")) LoadBloomConfig(root.GetMap("Bloom"), config.bloom);
  if (root.HasKey("SSAO")) LoadSSAOConfig(root.GetMap("SSAO"), config.ssao);
  if (root.HasKey("SMAA")) LoadSMAAConfig(root.GetMap("SMAA"), config.smaa);

  if (root.HasKey("SceneDefaults")) {
    LoadSceneDefaults(root.GetMap("SceneDefaults"), config.scene_defaults);
  }

  return config;
}
