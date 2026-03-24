#pragma once

#include <string>

#include "Framework/Core/color.h"
#include "Framework/Render/frame_packet.h"
#include "Framework/Render/render_handles.h"

class AssetManager;

class BackgroundSetting {
 public:
  void SetSkybox(const std::string& path, AssetManager& asset_manager);

  void SetClearColor(Color color) {
    mode_ = BackgroundMode::ClearColor;
    clear_color_ = color;
    cubemap_ = {};
  }

  void SetMode(BackgroundMode mode) {
    mode_ = mode;
  }
  void SetClearColorValue(Color color) {
    clear_color_ = color;
  }

  BackgroundConfig ToConfig() const;

  BackgroundMode GetMode() const {
    return mode_;
  }
  const Color& GetClearColor() const {
    return clear_color_;
  }
  bool HasSkybox() const {
    return cubemap_.IsValid();
  }
  const std::string& GetSkyboxPath() const {
    return skybox_path_;
  }

 private:
  BackgroundMode mode_ = BackgroundMode::ClearColor;
  Color clear_color_ = colors::DarkSlateGray;
  std::string skybox_path_;
  TextureHandle cubemap_;
};
