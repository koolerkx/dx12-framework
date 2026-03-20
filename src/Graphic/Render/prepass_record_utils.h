#pragma once

#include <functional>
#include <vector>

#include "Command/render_command_list.h"
#include "Frame/constant_buffers.h"
#include "Frame/resolved_draw_command.h"

using PrepassObjectCBBuilder = std::function<ObjectCB(const ResolvedDrawCommand&)>;

inline void RecordPrepassCommands(
  RenderCommandList& cmd,
  const std::vector<ResolvedDrawCommand>& commands,
  PrepassObjectCBBuilder build_obj_cb) {

  for (const auto& draw_cmd : commands) {
    if (!draw_cmd.material || draw_cmd.geometry.index_count == 0) continue;

    ObjectCB obj_data = build_obj_cb(draw_cmd);
    cmd.SetObjectConstants(obj_data);

    if (draw_cmd.instance_count > 1) {
      cmd.SetInstanceBufferSRV(draw_cmd.instance_buffer_address);
    }

    auto vbv = draw_cmd.geometry.vbv;
    auto ibv = draw_cmd.geometry.ibv;
    cmd.GetNative()->IASetVertexBuffers(0, 1, &vbv);
    cmd.GetNative()->IASetIndexBuffer(&ibv);
    cmd.GetNative()->DrawIndexedInstanced(
      draw_cmd.geometry.index_count, draw_cmd.instance_count,
      draw_cmd.geometry.index_offset, draw_cmd.geometry.vertex_offset, 0);
  }
}
