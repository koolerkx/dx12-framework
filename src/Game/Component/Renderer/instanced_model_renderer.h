#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Asset/model_data.h"
#include "Component/component.h"
#include "Framework/Math/Math.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Buffer/instance_buffer_handle.h"

using Math::Matrix4;
using Math::Vector4;

struct InstanceProps {
  Matrix4 world = Matrix4::Identity;
  Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
  Vector4 overlay_color = {0.0f, 0.0f, 0.0f, 0.0f};
  bool visible = true;
};

class InstancedModelRenderer : public Component<InstancedModelRenderer> {
 public:
  struct InstanceEntry {
    std::string id;
    InstanceProps props;
  };

  struct Props {
    std::shared_ptr<ModelData> model;
    std::vector<InstanceEntry> instances;
  };

  using Component::Component;
  InstancedModelRenderer(GameObject* owner, const Props& props);

  void OnInit() override;
  void OnRender(FramePacket& packet) override;
  void OnDestroy() override;

  void UpdateById(const std::string& id, std::function<InstanceProps(const InstanceProps&)> modifier);

 private:
  std::shared_ptr<ModelData> model_;
  std::vector<InstanceEntry> entries_;
  std::unordered_map<std::string, uint32_t> id_to_index_;

  InstanceBufferHandle buffer_handle_ = InstanceBufferHandle::Invalid;
  uint32_t instance_count_ = 0;
  bool dirty_ = true;
};
