/**
 * @file resolved_command_grouper.h
 * @brief Auto instance grouping for ResolvedDrawCommand.
 */
#pragma once

#include <vector>

#include "Frame/resolved_draw_command.h"

class DynamicUploadBuffer;

class ResolvedCommandGrouper {
 public:
  static void Group(std::vector<ResolvedDrawCommand>& commands, DynamicUploadBuffer* allocator);

  static void GroupForPrepass(std::vector<ResolvedDrawCommand>& commands, DynamicUploadBuffer* allocator);
};
