#include "material_pass.h"

#include "draw_command_resolver.h"

MaterialPass::MaterialPass(const MaterialPassProps& props)
    : name_(props.name), renderer_(props.renderer), layer_(props.layer), camera_provider_(props.camera) {
  setup_ = props.pass_setup;
}

void MaterialPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  CameraData camera = camera_provider_ ? camera_provider_(frame, packet) : packet.main_camera;

  DrawCommandResolver::ResolveContext ctx{
    .material_manager = frame.material_manager,
    .mesh_buffer_pool = frame.mesh_buffer_pool,
    .instance_allocator = frame.object_cb_allocator,
    .shadow_enabled = packet.shadow.enabled,
  };
  renderer_->BuildResolved(packet, layer_, ctx, resolved_cache_);

  renderer_->RecordResolvedCommands(
    frame, resolved_cache_, camera, packet.lighting, packet.shadow, frame.screen_width, frame.screen_height, packet.time);
}
