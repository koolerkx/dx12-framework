#pragma once

#include <vector>

#include "Command/render_command_list.h"
#include "Frame/resolved_draw_command.h"
#include "Render/object_index_utils.h"

inline void RecordPrepassCommands(
  RenderCommandList& cmd,
  const std::vector<ResolvedDrawCommand>& commands) {

  auto batch = BatchAllocateSingleObjectIndices(cmd, commands);

  for (const auto& draw_cmd : commands) {
    if (!draw_cmd.material || draw_cmd.geometry.index_count == 0) continue;

    auto vbv = draw_cmd.geometry.vbv;
    auto ibv = draw_cmd.geometry.ibv;
    cmd.GetNative()->IASetVertexBuffers(0, 1, &vbv);
    cmd.GetNative()->IASetIndexBuffer(&ibv);

    BindObjectIndexStream(cmd.GetNative(), draw_cmd, batch);

    cmd.GetNative()->DrawIndexedInstanced(
      draw_cmd.geometry.index_count, draw_cmd.instance_count,
      draw_cmd.geometry.index_offset, draw_cmd.geometry.vertex_offset, 0);
  }
}
