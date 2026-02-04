#include "render_services.h"

#include "Font/sprite_font_manager.h"
#include "Pipeline/material_manager.h"
#include "Pipeline/shader_manager.h"
#include "Texture/texture_manager.h"

#include "Framework/Logging/logger.h"

namespace gfx {

std::unique_ptr<RenderServices> RenderServices::Create(const CreateInfo& info) {
  auto services = std::unique_ptr<RenderServices>(new RenderServices());
  if (!services->Initialize(info)) {
    return nullptr;
  }
  return services;
}

RenderServices::~RenderServices() = default;

bool RenderServices::Initialize(const CreateInfo& info) {
  get_current_fence_value_ = info.get_current_fence_value;
  frame_buffer_count_ = info.frame_buffer_count;

  shader_manager_ = std::make_unique<ShaderManager>();
  if (!shader_manager_->Initialize(info.device)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize ShaderManager");
    return false;
  }

  material_manager_ = std::make_unique<MaterialManager>();
  if (!material_manager_->Initialize(info.device, shader_manager_.get())) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize MaterialManager");
    return false;
  }

  texture_manager_ = std::make_unique<TextureManager>();
  if (!texture_manager_->Initialize(
        info.device, info.heap_manager, info.execute_sync, info.get_current_fence_value, info.frame_buffer_count)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize TextureManager");
    return false;
  }

  font_manager_ = std::make_unique<Font::SpriteFontManager>();
  if (!font_manager_->Initialize(texture_manager_.get(), info.device)) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to initialize SpriteFontManager");
    return false;
  }
  Font::LoadDefaultFonts(*font_manager_);

  Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "RenderServices initialized");
  return true;
}

void RenderServices::OnFrameBegin([[maybe_unused]] uint32_t frame_index, uint64_t completed_fence) {
  texture_manager_->ProcessDeferredFrees(completed_fence);
}

void RenderServices::OnFrameEnd() {
  texture_manager_->CleanUploadBuffers();
  material_manager_->OnFrameEnd();
}

}  // namespace gfx
