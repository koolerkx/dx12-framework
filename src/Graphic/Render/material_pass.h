#pragma once

#include <functional>
#include <vector>

#include "Frame/camera_data.h"
#include "Frame/draw_command.h"
#include "Frame/frame_packet.h"
#include "Frame/render_frame_context.h"
#include "Frame/render_layer.h"
#include "material_renderer.h"
#include "render_pass.h"

class MaterialPass : public IRenderPass {
 public:
  using CameraProvider = std::function<CameraData(const RenderFrameContext&, const FramePacket&)>;

  struct MaterialPassProps {
    const char* name;
    MaterialRenderer* renderer;
    RenderLayer layer;
    PassSetup pass_setup;
    CameraProvider camera = {};
  };

  explicit MaterialPass(const MaterialPassProps& props);

  void Execute(const RenderFrameContext& frame, const FramePacket& packet) override;

  const char* GetName() const override {
    return name_;
  }

 private:
  const char* name_;
  MaterialRenderer* renderer_;
  RenderLayer layer_;
  CameraProvider camera_provider_;
  std::vector<DrawCommand> command_cache_;
};
