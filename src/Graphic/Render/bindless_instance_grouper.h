#pragma once

#include <vector>

#include "Frame/draw_command.h"

class DynamicUploadBuffer;

class BindlessInstanceGrouper {
 public:
  static void Group(std::vector<DrawCommand>& commands, DynamicUploadBuffer* allocator);
  static void GroupForPrepass(std::vector<DrawCommand>& commands, DynamicUploadBuffer* allocator);
};
