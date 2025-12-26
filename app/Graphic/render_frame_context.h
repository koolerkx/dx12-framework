#pragma once
#include <d3d12.h>

#include <cstdint>

#include "buffer.h"
#include "constant_buffers.h"
#include "descriptor_heap_allocator.h"
#include "descriptor_heap_manager.h"
#include "dynamic_upload_buffer.h"

struct RenderFrameContext {
  uint32_t frame_index;

  // The encapsulated command interface
  // Note: In a real engine, this might be a pointer or reference managed by the Graphic class
  // For now, we construct it on stack or store it here.
  // To keep it simple, let's store the raw pointers here,
  // but the users will wrap it in RenderCommandList immediately.

  ID3D12GraphicsCommandList* command_list;

  ConstantBuffer<FrameCB>* frame_cb;
  DynamicUploadBuffer* object_cb_allocator;

  DescriptorHeapAllocator* dynamic_allocator;
  DescriptorHeapManager* global_heap_manager;  // Needed to bind the global heap

  uint32_t screen_width;
  uint32_t screen_height;
};
