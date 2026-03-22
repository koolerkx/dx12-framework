#pragma once

#include <vector>

#include "Framework/Asset/asset_manager.h"
#include "Component/renderer_component.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/frame_packet.h"

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
  std::vector<MeshInstanceEntry> entries_;

  MaterialHandle material_handle_;
  std::vector<InstanceData> instance_cache_;
  uint32_t instance_count_ = 0;
};
