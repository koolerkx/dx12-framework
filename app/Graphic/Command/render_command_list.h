#pragma once
#include <DirectXMath.h>
#include <d3d12.h>

#include <vector>

#include "Command/buffer.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "Frame/constant_buffers.h"
#include "Frame/dynamic_upload_buffer.h"
#include "Pipeline/material.h"
#include "Pipeline/root_parameter_slots.h"
#include "Resource/mesh.h"

// Forward decls
struct Texture;
struct SpriteInstanceData;

class RenderCommandList {
 public:
  RenderCommandList(ID3D12GraphicsCommandList* cmd,
    DescriptorHeapAllocator* dynamic_heap,
    ConstantBuffer<FrameCB>* frame_cb,
    DynamicUploadBuffer* object_allocator)
      : cmd_(cmd),
        dynamic_heap_(dynamic_heap),
        frame_cb_(frame_cb),
        object_allocator_(object_allocator),
        current_material_(nullptr),
        current_root_signature_(nullptr) {
  }

  // Raw access if absolutely needed (try to avoid)
  ID3D12GraphicsCommandList* GetNative() const {
    return cmd_;
  }

  DescriptorHeapAllocator* GetDynamicHeap() const {
    return dynamic_heap_;
  }

  // --- High Level API ---

  // Set material (binds PSO and root signature)
  void SetMaterial(const Material* material) {
    if (!material || !material->IsValid()) {
      return;
    }

    ID3D12RootSignature* root_sig = material->GetRootSignature();
    if (current_root_signature_ != root_sig) {
      cmd_->SetGraphicsRootSignature(root_sig);
      current_root_signature_ = root_sig;
    }

    cmd_->SetPipelineState(material->GetPipelineState());
    current_material_ = material;
  }

  // Bind global descriptor heaps (can be called before root signature)
  void BindDescriptorHeaps(DescriptorHeapManager* manager) {
    manager->SetDescriptorHeaps(cmd_);
  }

  // Bind global SRV table (MUST be called after root signature is set)
  void BindGlobalSRVTable(DescriptorHeapManager* manager) {
    cmd_->SetGraphicsRootDescriptorTable(
      RootSlot::ToIndex(RootSlot::DescriptorTable::GlobalSRVs), manager->GetGlobalSrvHeap()->GetGPUDescriptorHandleForHeapStart());
  }

  void SetViewport(const D3D12_VIEWPORT& vp, const D3D12_RECT& sc) {
    cmd_->RSSetViewports(1, &vp);
    cmd_->RSSetScissorRects(1, &sc);
  }

  // Automatically uploads and binds Frame CB
  // Uses DynamicUploadBuffer to allocate new memory each time,
  void SetFrameConstants(const FrameCB& data) {
    auto allocation = object_allocator_->Allocate<FrameCB>();
    if (allocation.cpu_ptr == nullptr) {
      return;
    }
    memcpy(allocation.cpu_ptr, &data, sizeof(FrameCB));
    cmd_->SetGraphicsRootConstantBufferView(RootSlot::ToIndex(RootSlot::ConstantBuffer::Frame), allocation.gpu_ptr);
  }

  // Automatically uploads and binds Object CB
  void SetObjectConstants(const ObjectCB& data) {
    auto allocation = object_allocator_->Allocate<ObjectCB>();
    if (allocation.cpu_ptr == nullptr) {
      return;
    }
    memcpy(allocation.cpu_ptr, &data, sizeof(ObjectCB));
    cmd_->SetGraphicsRootConstantBufferView(RootSlot::ToIndex(RootSlot::ConstantBuffer::Object), allocation.gpu_ptr);
  }

  // Set material instance data (texture indices)
  void SetMaterialData(const MaterialInstance& material_instance) {
    // Pack material data into 32-bit constants
    uint32_t material_data[4] = {material_instance.albedo_texture_index,
      material_instance.normal_texture_index,
      material_instance.metallic_roughness_index,
      (material_instance.use_alpha_test ? 1u : 0u) | (material_instance.double_sided ? 2u : 0u)};

    cmd_->SetGraphicsRoot32BitConstants(RootSlot::ToIndex(RootSlot::Constants::MaterialData), 4, material_data, 0);
  }

  void DrawMesh(const Mesh* mesh) {
    if (mesh) {
      mesh->Draw(cmd_);
    }
  }

  // Draw mesh with hardware instancing (for batched rendering)
  void DrawMeshInstanced(const Mesh* mesh, const std::vector<SpriteInstanceData>& instances);

 private:
  ID3D12GraphicsCommandList* cmd_;
  DescriptorHeapAllocator* dynamic_heap_;
  ConstantBuffer<FrameCB>* frame_cb_;
  DynamicUploadBuffer* object_allocator_;
  const Material* current_material_ = nullptr;
  ID3D12RootSignature* current_root_signature_ = nullptr;
};
