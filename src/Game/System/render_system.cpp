#include "render_system.h"

#include "Component/renderer_component.h"
#include "Framework/Render/frame_packet.h"

void RenderSystem::Render(FramePacket& packet) {
  for (auto* renderable : renderables_) {
    if (!renderable->IsRenderEnabled()) continue;
    renderable->OnRender(packet);
  }
}

void RenderSystem::Register(IRenderable* renderable) {
  renderables_.push_back(renderable);
}

void RenderSystem::Unregister(IRenderable* renderable) {
  auto it = std::find(renderables_.begin(), renderables_.end(), renderable);
  if (it != renderables_.end()) {
    *it = renderables_.back();
    renderables_.pop_back();
  }
}

void RenderSystem::Clear() {
  renderables_.clear();
}
