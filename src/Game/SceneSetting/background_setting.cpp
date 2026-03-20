#include "background_setting.h"

#include "Asset/asset_manager.h"

void BackgroundSetting::SetSkybox(const std::string& path, AssetManager& asset_manager) {
  mode_ = BackgroundMode::Skybox;
  skybox_path_ = path;
  cubemap_ = asset_manager.LoadCubemap(path);
}

BackgroundConfig BackgroundSetting::ToConfig() const {
  BackgroundConfig config;
  config.mode = mode_;
  config.clear_color = clear_color_;
  if (cubemap_.IsValid()) {
    config.cubemap_srv_index = cubemap_.GetBindlessIndex();
  }
  return config;
}
