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

  void SetMode(BackgroundMode mode) { mode_ = mode; }
  void SetClearColorValue(Color color) { clear_color_ = color; }

  BackgroundConfig ToConfig() const;

  BackgroundMode GetMode() const { return mode_; }
  const Color& GetClearColor() const { return clear_color_; }
  bool HasSkybox() const { return cubemap_.Get() != nullptr; }

 private:
  BackgroundMode mode_ = BackgroundMode::ClearColor;
  Color clear_color_ = colors::DarkSlateGray;
  AssetHandle<Texture> cubemap_;
};
