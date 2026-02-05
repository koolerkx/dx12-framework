#include "ui_pass.h"

#include "material_renderer.h"

UiPass::UiPass(UiRenderer* renderer) : ui_renderer_(renderer) {
}

void UiPass::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  command_cache_.clear();
  ui_renderer_->Build(packet, RenderLayer::UI, command_cache_);

  // Create UI camera with orthographic projection for this frame
  using namespace DirectX;
  CameraData ui_camera;

  XMMATRIX view = XMMatrixIdentity();

  XMMATRIX proj = XMMatrixOrthographicOffCenterLH(0.0f,  // left
    static_cast<float>(frame.screen_width),              // right
    static_cast<float>(frame.screen_height),             // bottom
    0.0f,                                                // top
    -10.0f,                                              // near
    100.0f                                               // far
  );

  StoreMatrixToCameraData(ui_camera, view, proj);

  ui_camera.position = XMFLOAT3(0, 0, 0);
  ui_camera.forward = XMFLOAT3(0, 0, 1);
  ui_camera.up = XMFLOAT3(0, 1, 0);

  ui_renderer_->Record(frame, command_cache_, ui_camera, frame.screen_width, frame.screen_height);
}
