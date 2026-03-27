#pragma once

#include <functional>
#include <vector>

#include "Frame/render_frame_context.h"
#include "Frame/resolved_draw_command.h"
#include "Framework/Render/camera_data.h"
#include "Framework/Render/frame_packet.h"
#include "Framework/Render/render_types.h"
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
  std::vector<ResolvedDrawCommand> resolve_command_cache_;
};
