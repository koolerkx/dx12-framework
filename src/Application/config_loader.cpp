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

void LoadOutlineConfig(const framework::SerializeNode& node, OutlineConfig& outline) {
  outline.enabled = node.ReadBool("Enabled", outline.enabled);
  outline.depth_weight = node.ReadFloat("DepthWeight", outline.depth_weight);
  outline.normal_weight = node.ReadFloat("NormalWeight", outline.normal_weight);
  outline.edge_threshold = node.ReadFloat("EdgeThreshold", outline.edge_threshold);
  outline.depth_falloff = node.ReadFloat("DepthFalloff", outline.depth_falloff);
  outline.thickness = node.ReadFloat("Thickness", outline.thickness);
  node.ReadVec3("Color", outline.outline_color[0], outline.outline_color[1], outline.outline_color[2]);
}

void LoadFogConfig(const framework::SerializeNode& node, FogConfig& fog) {
  fog.enabled = node.ReadBool("Enabled", fog.enabled);
  fog.density = node.ReadFloat("Density", fog.density);
  fog.height_falloff = node.ReadFloat("HeightFalloff", fog.height_falloff);
  fog.base_height = node.ReadFloat("BaseHeight", fog.base_height);
  fog.max_distance = node.ReadFloat("MaxDistance", fog.max_distance);
  node.ReadVec3("Color", fog.fog_color[0], fog.fog_color[1], fog.fog_color[2]);
}

void LoadVignetteConfig(const framework::SerializeNode& node, VignetteConfig& vignette) {
  vignette.enabled = node.ReadBool("Enabled", vignette.enabled);
  vignette.intensity = node.ReadFloat("Intensity", vignette.intensity);
  vignette.radius = node.ReadFloat("Radius", vignette.radius);
  vignette.softness = node.ReadFloat("Softness", vignette.softness);
  vignette.roundness = node.ReadFloat("Roundness", vignette.roundness);
  node.ReadVec3("Color", vignette.vignette_color[0], vignette.vignette_color[1], vignette.vignette_color[2]);
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
  config.debug_draw_enabled = root.ReadBool("DebugDraw", config.debug_draw_enabled);
  config.enable_debug_layer_for_debug_build_only =
    root.ReadBool("EnableDebugLayerForDebugBuildOnly", config.enable_debug_layer_for_debug_build_only);
  config.startup_scene = root.ReadString("StartupScene", config.startup_scene);

  if (root.HasKey("Bloom")) LoadBloomConfig(root.GetMap("Bloom"), config.bloom);
  if (root.HasKey("SSAO")) LoadSSAOConfig(root.GetMap("SSAO"), config.ssao);
  if (root.HasKey("SMAA")) LoadSMAAConfig(root.GetMap("SMAA"), config.smaa);
  if (root.HasKey("Fog")) LoadFogConfig(root.GetMap("Fog"), config.fog);
  if (root.HasKey("Outline")) LoadOutlineConfig(root.GetMap("Outline"), config.outline);
  if (root.HasKey("Vignette")) LoadVignetteConfig(root.GetMap("Vignette"), config.vignette);

  if (root.HasKey("SceneDefaults")) {
    LoadSceneDefaults(root.GetMap("SceneDefaults"), config.scene_defaults);
  }

  if (root.HasKey("Editor")) {
    auto editor = root.GetMap("Editor");
    editor.ReadVec4(
      "BackgroundColor", config.editor_bg_color[0], config.editor_bg_color[1], config.editor_bg_color[2], config.editor_bg_color[3]);
  }

  return config;
}
