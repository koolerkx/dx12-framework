#include "material_pass.h"

MaterialPass::MaterialPass(const MaterialPassProps& props)
    : name_(props.name), renderer_(props.renderer), layer_(props.layer), camera_provider_(props.camera) {
  setup_ = props.pass_setup;
}

void MaterialPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  command_cache_.clear();
  renderer_->Build(packet, layer_, command_cache_);

  CameraData camera = camera_provider_ ? camera_provider_(frame, packet) : packet.main_camera;

  renderer_->Record(frame, command_cache_, camera, packet.lighting, frame.screen_width, frame.screen_height);
}
