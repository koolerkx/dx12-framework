/**
 * @file object_index_utils.h
 * @brief Utilities for unified objectIndex buffer, indirect draw args, and ExecuteIndirect dispatch.
 */
#pragma once

#include <cstring>
#include <vector>

#include "Command/render_command_list.h"
#include "Frame/resolved_draw_command.h"

struct UnifiedIndexBuffer {
  D3D12_GPU_VIRTUAL_ADDRESS gpu_address = 0;
  uint32_t total_count = 0;
};
// Build a single contiguous objectIndex buffer for all draws.
// Assigns start_instance_location on each command.
inline UnifiedIndexBuffer BuildUnifiedObjectIndexBuffer(RenderCommandList& cmd, std::vector<ResolvedDrawCommand>& commands) {
  uint32_t total = 0;
  for (const auto& c : commands) {
    if (!c.IsDrawable()) continue;
    total += c.instance_count;
  }

  if (total == 0) return {};

  auto alloc = cmd.AllocateUpload(total * sizeof(uint32_t));
  auto* buf = static_cast<uint32_t*>(alloc.cpu_ptr);

  uint32_t offset = 0;
  for (auto& c : commands) {
    if (!c.IsDrawable()) continue;

    c.start_instance_location = offset;

    if (!c.grouped_object_indices.empty()) {
      std::memcpy(buf + offset, c.grouped_object_indices.data(), c.grouped_object_indices.size() * sizeof(uint32_t));
    } else if (c.instance_count == 1) {
      buf[offset] = c.object_index;
    } else {
      for (uint32_t i = 0; i < c.instance_count; ++i) {
        buf[offset + i] = c.object_index + i;
      }
    }

    offset += c.instance_count;
  }

  return {alloc.gpu_ptr, total};
}

inline void BindUnifiedObjectIndexBuffer(ID3D12GraphicsCommandList* cmd_list, const UnifiedIndexBuffer& buffer) {
  if (!buffer.gpu_address) return;
  D3D12_VERTEX_BUFFER_VIEW vbv = {buffer.gpu_address, static_cast<UINT>(buffer.total_count * sizeof(uint32_t)), sizeof(uint32_t)};
  cmd_list->IASetVertexBuffers(1, 1, &vbv);
}

// Build indirect args and dispatch ExecuteIndirect for a range of commands with the same PSO.
// custom_data draws are skipped (caller handles them separately).
inline void EmitExecuteIndirect(RenderCommandList& cmd,
  const std::vector<ResolvedDrawCommand>& commands,
  size_t start,
  size_t end,
  ID3D12CommandSignature* command_signature) {
  // Single pass: allocate upper-bound, fill args, track actual count
  size_t max_count = end - start;
  auto args_alloc = cmd.AllocateUpload(max_count * sizeof(D3D12_DRAW_INDEXED_ARGUMENTS));
  auto* args = static_cast<D3D12_DRAW_INDEXED_ARGUMENTS*>(args_alloc.cpu_ptr);

  uint32_t indirect_count = 0;
  for (size_t i = start; i < end; ++i) {
    if (!commands[i].IsDrawable()) continue;
    if (commands[i].custom_data.active) continue;
    args[indirect_count++] = {commands[i].geometry.index_count,
      commands[i].instance_count,
      commands[i].geometry.index_offset,
      static_cast<INT>(commands[i].geometry.vertex_offset),
      commands[i].start_instance_location};
  }

  if (indirect_count > 0) {
    auto vbv = commands[start].geometry.vbv;
    auto ibv = commands[start].geometry.ibv;
    cmd.GetNative()->IASetVertexBuffers(0, 1, &vbv);
    cmd.GetNative()->IASetIndexBuffer(&ibv);
    cmd.GetNative()->ExecuteIndirect(command_signature, indirect_count, args_alloc.resource, args_alloc.offset_in_resource, nullptr, 0);
  }
}
