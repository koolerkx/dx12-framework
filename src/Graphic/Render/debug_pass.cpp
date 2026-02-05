#include "debug_pass.h"

#include "Graphic/Debug/debug_line_renderer.h"
#include "Graphic/Pipeline/material_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"

DebugPass::DebugPass(DebugLineRenderer* renderer, MaterialManager* material_mgr, PassSetup pass_setup)
    : debug_line_renderer_(renderer), material_manager_(material_mgr) {
  setup_ = std::move(pass_setup);
}

void DebugPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  if (!debug_line_renderer_ || !material_manager_) return;

  Rendering::RenderSettings settings = MaterialManager::GetDefaultSettings(Graphics::DebugLineShader::ID);

  const Material* line_material = material_manager_->GetOrCreateMaterial<Graphics::DebugLineShader>(settings);
  if (!line_material) return;

  debug_line_renderer_->Render(frame, line_material, packet.main_camera.view_proj);
}
