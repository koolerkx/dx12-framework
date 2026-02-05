#include "material_pass.h"

MaterialPass::MaterialPass(const char* name, MaterialRenderer* renderer, RenderLayer layer, PassSetup pass_setup, CameraProvider camera)
    : name_(name), renderer_(renderer), layer_(layer), camera_provider_(std::move(camera)) {
  setup_ = std::move(pass_setup);
}

void MaterialPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  command_cache_.clear();
  renderer_->Build(packet, layer_, command_cache_);

  CameraData camera = camera_provider_ ? camera_provider_(frame, packet) : packet.main_camera;

  renderer_->Record(frame, command_cache_, camera, frame.screen_width, frame.screen_height);
}
