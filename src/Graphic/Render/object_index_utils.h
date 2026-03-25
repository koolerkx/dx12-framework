/**
 * @file object_index_utils.h
 * @brief Utilities for objectIndex IA slot 1 binding.
 */
#pragma once

#include <vector>

#include "Command/render_command_list.h"
#include "Frame/resolved_draw_command.h"

struct BatchIndexAllocation {
  D3D12_GPU_VIRTUAL_ADDRESS base_gpu = 0;
  uint32_t count = 0;
  uint32_t next_slot = 0;
};

// Batch-allocate objectIndex values for all single draws into one contiguous buffer.
// Avoids per-draw 256B-aligned allocation (4B data per draw, 256B waste each).
inline BatchIndexAllocation BatchAllocateSingleObjectIndices(
    RenderCommandList& cmd,
    const std::vector<ResolvedDrawCommand>& commands) {

  uint32_t single_count = 0;
  for (const auto& c : commands) {
    if (c.instance_count <= 1 && c.object_index != UINT32_MAX && c.material) {
      ++single_count;
    }
  }

  if (single_count == 0) return {};

  auto alloc = cmd.AllocateUpload(single_count * sizeof(uint32_t));
  auto* indices = static_cast<uint32_t*>(alloc.cpu_ptr);

  uint32_t slot = 0;
  for (const auto& c : commands) {
    if (c.instance_count <= 1 && c.object_index != UINT32_MAX && c.material) {
      indices[slot++] = c.object_index;
    }
  }

  return {alloc.gpu_ptr, single_count, 0};
}

inline void BindObjectIndexStream(
    ID3D12GraphicsCommandList* cmd_list,
    const ResolvedDrawCommand& draw_cmd,
    BatchIndexAllocation& batch) {

  if (draw_cmd.instance_count > 1) {
    D3D12_VERTEX_BUFFER_VIEW index_vbv = {
      draw_cmd.instance_buffer_address,
      static_cast<UINT>(draw_cmd.instance_count * sizeof(uint32_t)),
      sizeof(uint32_t)
    };
    cmd_list->IASetVertexBuffers(1, 1, &index_vbv);
  } else if (draw_cmd.object_index != UINT32_MAX && batch.base_gpu != 0) {
    D3D12_VERTEX_BUFFER_VIEW index_vbv = {
      batch.base_gpu + batch.next_slot * sizeof(uint32_t),
      sizeof(uint32_t),
      sizeof(uint32_t)
    };
    cmd_list->IASetVertexBuffers(1, 1, &index_vbv);
    ++batch.next_slot;
  }
}
