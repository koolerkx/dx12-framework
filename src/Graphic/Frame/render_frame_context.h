#pragma once
#include <d3d12.h>

#include <cstdint>

#include "Command/buffer.h"
#include "Descriptor/descriptor_heap_allocator.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "Frame/constant_buffers.h"
#include "Frame/dynamic_upload_buffer.h"
#include "Framework/Math/Math.h"

class RenderGraph;

struct ShadowFrameData {
  Math::Matrix4 light_view_proj;
  uint32_t shadow_map_srv_index = UINT32_MAX;
  uint32_t shadow_map_resolution = 2048;
};

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
  ShadowFrameData* shadow_data = nullptr;
};
