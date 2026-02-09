#pragma once
#include <d3d12.h>

#include <cstdint>

#include "Command/buffer.h"
#include "Descriptor/descriptor_heap_allocator.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "Frame/constant_buffers.h"
#include "Frame/dynamic_upload_buffer.h"

class RenderGraph;

struct RenderFrameContext {
  uint32_t frame_index;

  ID3D12GraphicsCommandList* command_list;

  ConstantBuffer<FrameCB>* frame_cb;
  DynamicUploadBuffer* object_cb_allocator;

  DescriptorHeapAllocator* dynamic_allocator;
  DescriptorHeapManager* global_heap_manager;

  uint32_t screen_width;
  uint32_t screen_height;

  RenderGraph* render_graph = nullptr;
};
