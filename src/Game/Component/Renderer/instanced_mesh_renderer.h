#pragma once

#include <vector>

#include "Asset/asset_manager.h"
#include "Component/renderer_component.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/render_handles.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Buffer/instance_buffer_handle.h"

using Math::Matrix4;
using Math::Vector4;

struct MeshInstanceEntry {
  Matrix4 world = Matrix4::Identity;
  Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
  Vector4 overlay_color = {0.0f, 0.0f, 0.0f, 0.0f};
};

class InstancedMeshRenderer : public RendererComponent<InstancedMeshRenderer> {
 public:
  struct Props {
    DefaultMesh mesh_type = DefaultMesh::Cube;
    std::vector<MeshInstanceEntry> instances;
  };

  using RendererComponent::RendererComponent;
  InstancedMeshRenderer(GameObject* owner, const Props& props);

  void OnInit() override;
  void OnRender(FramePacket& packet) override;
  void OnDestroy() override;

 private:
  DefaultMesh mesh_type_;
  const Mesh* mesh_ = nullptr;
  std::vector<MeshInstanceEntry> entries_;

  InstanceBufferHandle buffer_handle_ = InstanceBufferHandle::Invalid;
  MaterialHandle material_handle_;
  uint32_t instance_count_ = 0;
  bool dirty_ = true;
};
