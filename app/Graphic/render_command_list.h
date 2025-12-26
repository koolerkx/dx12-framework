#pragma once
#include <DirectXMath.h>
#include <d3d12.h>

#include "buffer.h"
#include "constant_buffers.h"
#include "descriptor_heap_manager.h"
#include "dynamic_upload_buffer.h"
#include "mesh.h"

// Forward decls
struct Texture;

class RenderCommandList {
 public:
  RenderCommandList(ID3D12GraphicsCommandList* cmd,
    DescriptorHeapAllocator* dynamic_heap,
    ConstantBuffer<FrameCB>* frame_cb,
    DynamicUploadBuffer* object_allocator_)
      : cmd_(cmd), dynamic_heap_(dynamic_heap), frame_cb_(frame_cb), object_allocator_(object_allocator_) {
  }

  // Raw access if absolutely needed (try to avoid)
  ID3D12GraphicsCommandList* GetNative() const {
    return cmd_;
  }
  DescriptorHeapAllocator* GetDynamicHeap() const {
    return dynamic_heap_;
  }

  // --- High Level API ---

  void SetPipelineState(ID3D12RootSignature* rootSig, ID3D12PipelineState* pso) {
    cmd_->SetGraphicsRootSignature(rootSig);
    cmd_->SetPipelineState(pso);
  }

  void BindHeap(DescriptorHeapManager* manager) {
    // We bind the global heap provided by manager
    manager->SetDescriptorHeaps(cmd_);
    // Bind the Global Table (Slot 3 in your RootSignature)
    cmd_->SetGraphicsRootDescriptorTable(3, manager->GetGlobalSrvHeap()->GetGPUDescriptorHandleForHeapStart());
  }

  void SetViewport(const D3D12_VIEWPORT& vp, const D3D12_RECT& sc) {
    cmd_->RSSetViewports(1, &vp);
    cmd_->RSSetScissorRects(1, &sc);
  }

  // Automatically uploads and binds Frame CB
  void SetFrameConstants(const FrameCB& data) {
    frame_cb_->Update(data);
    cmd_->SetGraphicsRootConstantBufferView(0, frame_cb_->GetGPUAddress());
  }

  // Automatically uploads and binds Object CB
  void SetObjectConstants(const ObjectCB& data) {
    auto allocation = object_allocator_->Allocate<ObjectCB>();

    if (allocation.cpu_ptr == nullptr) {
      return;
    }

    memcpy(allocation.cpu_ptr, &data, sizeof(ObjectCB));
    cmd_->SetGraphicsRootConstantBufferView(1, allocation.gpu_ptr);
  }

  // Bind texture by index (Bindless style)
  void BindTextureIndex(uint32_t rootParamIndex, uint32_t srvIndex) {
    cmd_->SetGraphicsRoot32BitConstants(rootParamIndex, 1, &srvIndex, 0);
  }

  void DrawMesh(Mesh* mesh) {
    mesh->Draw(cmd_);
  }

 private:
  ID3D12GraphicsCommandList* cmd_;
  DescriptorHeapAllocator* dynamic_heap_;
  ConstantBuffer<FrameCB>* frame_cb_;
  DynamicUploadBuffer* object_allocator_;
};
