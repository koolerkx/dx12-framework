#include "material_pass.h"

#include "draw_command_resolver.h"

MaterialPass::MaterialPass(const MaterialPassProps& props)
    : name_(props.name), renderer_(props.renderer), layer_(props.layer), camera_provider_(props.camera) {
  setup_ = props.pass_setup;
}

void MaterialPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  CameraData camera = camera_provider_ ? camera_provider_(frame, packet) : packet.main_camera;

  command_cache_.clear();
  renderer_->Build(packet, layer_, command_cache_, frame.object_cb_allocator);

  bool has_new_path = !packet.single_requests.empty() || !packet.instanced_requests.empty();
  if (has_new_path) {
    DrawCommandResolver::ResolveContext ctx{
      .material_manager = frame.material_manager,
      .mesh_buffer_pool = frame.mesh_buffer_pool,
      .instance_allocator = frame.object_cb_allocator,
      .shadow_enabled = packet.shadow.enabled,
    };
    resolved_cache_.clear();
    renderer_->BuildResolved(packet, layer_, ctx, resolved_cache_);
  }

  // During coexistence: merge old + new into a single depth-sorted record pass.
  // Old commands drawn via Record, new commands via RecordResolvedCommands.
  if (!resolved_cache_.empty() && !command_cache_.empty()) {
    renderer_->RecordMerged(
      frame, command_cache_, resolved_cache_, camera, packet.lighting, packet.shadow, frame.screen_width, frame.screen_height, packet.time);
  } else if (!resolved_cache_.empty()) {
    renderer_->RecordResolvedCommands(
      frame, resolved_cache_, camera, packet.lighting, packet.shadow, frame.screen_width, frame.screen_height, packet.time);
  } else {
    renderer_->Record(frame, command_cache_, camera, packet.lighting, packet.shadow, frame.screen_width, frame.screen_height, packet.time);
  }
}
