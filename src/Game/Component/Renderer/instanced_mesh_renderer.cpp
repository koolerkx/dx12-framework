#include "instanced_mesh_renderer.h"

#include "Component/render_settings.h"
#include "Framework/Logging/logger.h"
#include "Graphic/Resource/Buffer/instance_buffer_manager.h"
#include "Graphic/Resource/Material/material_descriptor_pool.h"
#include "game_context.h"
#include "game_object.h"

InstancedMeshRenderer::InstancedMeshRenderer(GameObject* owner, const Props& props)
    : RendererComponent(owner), mesh_type_(props.mesh_type), entries_(props.instances) {
  instance_count_ = static_cast<uint32_t>(entries_.size());
}

void InstancedMeshRenderer::OnInit() {
  RendererComponent::OnInit();
  if (instance_count_ == 0) return;

  auto* context = GetOwner()->GetContext();
  mesh_ = context->GetAssetManager().GetDefaultMesh(mesh_type_);
  if (!mesh_) return;

  auto& manager = context->GetGraphic()->GetInstanceBufferManager();
  buffer_handle_ = manager.Create(instance_count_);

  if (buffer_handle_ == InstanceBufferHandle::Invalid) {
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Game,
      Logger::Here(),
      "[InstancedMeshRenderer] Failed to create instance buffer for {} instances",
      instance_count_);
  }
}

void InstancedMeshRenderer::OnRender(FramePacket& packet) {
  if (!mesh_ || instance_count_ == 0 || buffer_handle_ == InstanceBufferHandle::Invalid) return;

  auto* graphic = GetOwner()->GetContext()->GetGraphic();
  auto& manager = graphic->GetInstanceBufferManager();

  if (dirty_) {
    std::vector<GPUInstanceData> gpu_data(instance_count_);
    for (uint32_t i = 0; i < instance_count_; ++i) {
      gpu_data[i].world = entries_[i].world;
      gpu_data[i].normal_matrix = entries_[i].world.Inverted().Transposed();
      gpu_data[i].color = entries_[i].color;
      gpu_data[i].overlay_color = entries_[i].overlay_color;
    }
    manager.Update(buffer_handle_, gpu_data.data(), instance_count_);
    dirty_ = false;
  }

  auto& material_mgr = graphic->GetMaterialManager();
  auto& pool = graphic->GetMaterialDescriptorPool();
  D3D12_GPU_VIRTUAL_ADDRESS buffer_address = manager.GetAddress(buffer_handle_);

  if (!material_handle_.IsValid()) {
    auto* context = GetOwner()->GetContext();
    Texture* albedo = context->GetAssetManager().GetDefaultWhiteTexture();
    MaterialDescriptor desc{};
    desc.albedo_texture_index = albedo ? albedo->GetBindlessIndex() : 0;
    desc.sampler_index = static_cast<uint32_t>(Rendering::SamplerType::AnisotropicWrap);
    material_handle_ = pool.Allocate(desc);
  }

  DrawCommand cmd;
  cmd.mesh = mesh_;
  cmd.instance_buffer_address = buffer_address;
  cmd.instance_count = instance_count_;

  auto settings = Rendering::RenderSettings::Opaque();
  cmd.material = material_mgr.GetOrCreateMaterial(Graphics::PBRShader::ID, settings);
  cmd.material_handle = material_handle_;

  cmd.layer = RenderLayer::Opaque;
  cmd.tags = static_cast<uint32_t>(RenderTag::CastShadow | RenderTag::ReceiveShadow | RenderTag::Lit);
  cmd.depth = 0.0f;

  packet.AddCommand(std::move(cmd));
}

void InstancedMeshRenderer::OnDestroy() {
  if (material_handle_.IsValid()) {
    auto* graphic = GetOwner()->GetContext()->GetGraphic();
    if (graphic) {
      graphic->GetMaterialDescriptorPool().Free(material_handle_);
    }
    material_handle_ = MaterialHandle::Invalid();
  }
  if (buffer_handle_ != InstanceBufferHandle::Invalid) {
    auto* graphic = GetOwner()->GetContext()->GetGraphic();
    uint64_t fence_value = graphic->GetCurrentFenceValue();
    graphic->GetInstanceBufferManager().Destroy(buffer_handle_, fence_value);
    buffer_handle_ = InstanceBufferHandle::Invalid;
  }
  RendererComponent::OnDestroy();
}
