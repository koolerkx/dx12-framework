#pragma once

#include <vector>

#include "Command/render_command_list.h"
#include "Frame/render_frame_context.h"
#include "Frame/resolved_draw_command.h"
#include "Render/object_index_utils.h"

inline void RecordPrepassCommands(RenderCommandList& cmd, std::vector<ResolvedDrawCommand>& commands, const RenderFrameContext& frame) {
  auto unified = BuildUnifiedObjectIndexBuffer(cmd, commands);
  BindUnifiedObjectIndexBuffer(cmd.GetNative(), unified);

  if (frame.command_signature) {
    EmitExecuteIndirect(cmd, commands, 0, commands.size(), frame.command_signature);
  } else {
    for (const auto& draw_cmd : commands) {
      if (!draw_cmd.IsDrawable()) continue;

      auto vbv = draw_cmd.geometry.vbv;
      auto ibv = draw_cmd.geometry.ibv;
      cmd.GetNative()->IASetVertexBuffers(0, 1, &vbv);
      cmd.GetNative()->IASetIndexBuffer(&ibv);
      cmd.GetNative()->DrawIndexedInstanced(draw_cmd.geometry.index_count,
        draw_cmd.instance_count,
        draw_cmd.geometry.index_offset,
        draw_cmd.geometry.vertex_offset,
        draw_cmd.start_instance_location);
    }
  }
}
