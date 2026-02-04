#include "debug_pass.h"

#include "Graphic/Debug/debug_line_renderer.h"
#include "Graphic/Pipeline/material_manager.h"
#include "Graphic/Pipeline/shader_descriptors.h"

DebugPass::DebugPass(DebugLineRenderer* renderer, MaterialManager* material_mgr)
    : debug_line_renderer_(renderer), material_manager_(material_mgr) {
}

void DebugPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  if (!debug_line_renderer_ || !material_manager_) return;

  // Get default settings for debug line shader
  Rendering::RenderSettings settings = MaterialManager::GetDefaultSettings(Graphics::DebugLineShader::ID);

  // Get debug line material using unified API
  const Material* line_material = material_manager_->GetOrCreateMaterial<Graphics::DebugLineShader>(settings);
  if (!line_material) return;

  // Render all accumulated debug lines
  debug_line_renderer_->Render(frame, line_material, packet.main_camera.view_proj);
}
