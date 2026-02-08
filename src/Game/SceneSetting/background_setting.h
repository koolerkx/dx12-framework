#pragma once

#include "Asset/asset_handle.h"
#include "Framework/Core/color.h"
#include "Graphic/Frame/frame_packet.h"

class AssetManager;
struct Texture;

class BackgroundSetting {
 public:
  void SetSkybox(const std::string& path, AssetManager& asset_manager);

  void SetClearColor(Color color) {
    mode_ = BackgroundMode::ClearColor;
    clear_color_ = color;
    cubemap_ = {};
  }

  BackgroundConfig ToConfig() const;

 private:
  BackgroundMode mode_ = BackgroundMode::ClearColor;
  Color clear_color_ = colors::DarkSlateGray;
  AssetHandle<Texture> cubemap_;
};
