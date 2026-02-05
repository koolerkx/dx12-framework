#include "render_pass_manager.h"

#include "Core/utils.h"
#include "Presentation/depth_buffer.h"
#include "Presentation/swapchain_manager.h"
#include "Render/render_texture.h"

void RenderPassManager::SetSwapChain(SwapChainManager* swapchain) {
  swapchain_ = swapchain;
}

void RenderPassManager::SetHeapManager(DescriptorHeapManager* heap_mgr) {
  heap_manager_ = heap_mgr;
}

RenderTexture* RenderPassManager::CreateRenderTexture(
  DXGI_FORMAT format, uint32_t width, uint32_t height, ID3D12Device* device, std::array<float, 4> clear_color) {
  auto rt = std::make_unique<RenderTexture>(format, clear_color);
  if (!rt->Initialize(device, width, height, *heap_manager_)) {
    return nullptr;
  }
  auto* ptr = rt.get();
  render_textures_.push_back(std::move(rt));
  return ptr;
}

DepthBuffer* RenderPassManager::CreateDepthBuffer(uint32_t width, uint32_t height, ID3D12Device* device) {
  depth_buffer_ = std::make_unique<DepthBuffer>();
  if (!depth_buffer_->Initialize(device, width, height, *heap_manager_)) {
    depth_buffer_.reset();
    return nullptr;
  }
  return depth_buffer_.get();
}

void RenderPassManager::AddPass(std::unique_ptr<IRenderPass> pass) {
  passes_.push_back(std::move(pass));
}

void RenderPassManager::BeginFrame() {
  backbuffer_is_render_target_ = false;
  backbuffer_cleared_ = false;

  for (auto& rt : render_textures_) {
    rt->ResetFrameState();
  }
  if (depth_buffer_) {
    depth_buffer_->ResetFrameState();
  }
}

void RenderPassManager::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  for (auto& pass : passes_) {
    if (!pass) continue;

    utils::CommandListEventGroup(frame.command_list, utils::utf8_to_wstring(pass->GetName()).c_str(), [&]() {
      ApplyPassSetup(frame.command_list, pass->GetPassSetup());
      pass->Execute(frame, packet);
    });
  }

  FinalizeFrame(frame.command_list);
}

void RenderPassManager::ApplyPassSetup(ID3D12GraphicsCommandList* cmd, const PassSetup& setup) {
  // Transition shader inputs to SRV
  for (auto* input : setup.shader_inputs) {
    if (input) {
      input->TransitionTo(cmd, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }
  }

  // Collect RTV handles and resolve viewport dimensions from first color target
  std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtv_handles;
  uint32_t rt_width = 0;
  uint32_t rt_height = 0;

  for (const auto& target : setup.color_targets) {
    if (target.texture == nullptr) {
      // Backbuffer
      if (!backbuffer_is_render_target_) {
        swapchain_->TransitionToRenderTarget(cmd);
        backbuffer_is_render_target_ = true;
      }
      if (!backbuffer_cleared_) {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapchain_->GetCurrentRTV();
        constexpr float clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        cmd->ClearRenderTargetView(rtv, clear_color, 0, nullptr);
        backbuffer_cleared_ = true;
      }
      rtv_handles.push_back(swapchain_->GetCurrentRTV());
      if (rt_width == 0) {
        rt_width = swapchain_->GetWdith();
        rt_height = swapchain_->GetHeight();
      }
    } else {
      // RenderTexture
      target.texture->TransitionTo(cmd, D3D12_RESOURCE_STATE_RENDER_TARGET);
      if (target.texture->NeedsClear()) {
        target.texture->Clear(cmd);
      }
      rtv_handles.push_back(target.texture->GetRTV());
      if (rt_width == 0) {
        rt_width = target.texture->GetWidth();
        rt_height = target.texture->GetHeight();
      }
    }
  }

  // Depth
  D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
  bool has_depth = setup.depth.buffer != nullptr;
  if (has_depth) {
    if (setup.depth.buffer->NeedsClear()) {
      setup.depth.buffer->Clear(cmd);
    }
    dsv = setup.depth.buffer->GetDSV();
  }

  // Bind render targets
  if (!rtv_handles.empty()) {
    cmd->OMSetRenderTargets(static_cast<UINT>(rtv_handles.size()), rtv_handles.data(), FALSE, has_depth ? &dsv : nullptr);
  }

  // Viewport and scissor derived from RT dimensions
  if (!rtv_handles.empty()) {
    D3D12_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(rt_width);
    viewport.Height = static_cast<float>(rt_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissor = {};
    scissor.right = static_cast<LONG>(rt_width);
    scissor.bottom = static_cast<LONG>(rt_height);

    cmd->RSSetViewports(1, &viewport);
    cmd->RSSetScissorRects(1, &scissor);
  }
}

void RenderPassManager::FinalizeFrame(ID3D12GraphicsCommandList* cmd) {
  if (backbuffer_is_render_target_) {
    swapchain_->TransitionToPresent(cmd);
    backbuffer_is_render_target_ = false;
  }
}

void RenderPassManager::Resize(ID3D12Device* device, uint32_t width, uint32_t height) {
  for (auto& rt : render_textures_) {
    rt->SafeRelease(*heap_manager_);
    rt->Initialize(device, width, height, *heap_manager_);
  }
  if (depth_buffer_) {
    depth_buffer_->SafeRelease();
    depth_buffer_->Initialize(device, width, height, *heap_manager_);
  }
}

void RenderPassManager::Shutdown() {
  passes_.clear();

  for (auto& rt : render_textures_) {
    rt->SafeRelease(*heap_manager_);
  }
  render_textures_.clear();

  if (depth_buffer_) {
    depth_buffer_->SafeRelease();
    depth_buffer_.reset();
  }
}
