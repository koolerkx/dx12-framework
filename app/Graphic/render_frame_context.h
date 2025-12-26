#pragma once
#include <d3d12.h>

#include <cstdint>

#include "buffer.h"
#include "constant_buffers.h"


class DescriptorHeapManager;

struct RenderFrameContext {
  uint32_t frame_index;
  ID3D12GraphicsCommandList* command_list;
  ID3D12Device* device;
  DescriptorHeapManager* descriptor_manager;

  ConstantBuffer<FrameCB>* frame_cb;
  ConstantBuffer<ObjectCB>* object_cb;
  uint32_t screen_width;
  uint32_t screen_height;
};
