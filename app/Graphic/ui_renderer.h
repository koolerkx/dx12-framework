#pragma once
#include <d3d12.h>

#include <vector>

#include "render_frame_context.h"
#include "render_world.h"

struct UiDrawPacket {
  float2 pos;
  float2 size;
  TextureHandle tex;
  float4 color;
};

template <typename T>
class ConstantBuffer;

struct FrameCB;
struct ObjectCB;
class Mesh;

class UiRenderer {
 public:
  UiRenderer(ID3D12RootSignature* root_sig, ID3D12PipelineState* pso, Mesh* quad_mesh);

  void Build(const RenderWorld& world, std::vector<UiDrawPacket>& out);
  void Record(const RenderFrameContext& frame,
    const std::vector<UiDrawPacket>& packets,
    ConstantBuffer<FrameCB>* frame_cb,
    ConstantBuffer<ObjectCB>* obj_cb,
    uint32_t screen_width,
    uint32_t screen_height);

 private:
  ID3D12RootSignature* root_signature_;
  ID3D12PipelineState* pipeline_state_;
  Mesh* quad_mesh_;
};
