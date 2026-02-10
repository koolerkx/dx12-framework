#include "render_graph.h"

#include <algorithm>
#include <cassert>
#include <queue>
#include <ranges>
#include <unordered_set>

#include "Core/utils.h"
#include "Descriptor/descriptor_heap_manager.h"
#include "Framework/Logging/logger.h"
#include "Presentation/depth_buffer.h"
#include "Presentation/swapchain_manager.h"
#include "Render/render_texture.h"

RenderGraph::RenderGraph() = default;
RenderGraph::~RenderGraph() = default;

void RenderGraph::SetSwapChain(SwapChainManager* swapchain) {
  swapchain_ = swapchain;
}

void RenderGraph::SetHeapManager(DescriptorHeapManager* heap_mgr) {
  heap_manager_ = heap_mgr;
}

RenderGraphHandle RenderGraph::CreateRenderTexture(
  const char* name, DXGI_FORMAT format, uint32_t width, uint32_t height, ID3D12Device* device, Color clear_color) {
  auto rt = std::make_unique<RenderTexture>(format, clear_color);
  if (!rt->Initialize(device, width, height, *heap_manager_)) {
    return RenderGraphHandle::Invalid;
  }
  rt->SetDebugName(name);

  auto index = static_cast<uint16_t>(resources_.size());
  resources_.push_back({name, RenderGraphResourceType::RenderTexture, rt.get(), nullptr});
  owned_render_textures_.push_back(std::move(rt));
  return static_cast<RenderGraphHandle>(index);
}

RenderGraphHandle RenderGraph::CreateDepthBuffer(const char* name, uint32_t width, uint32_t height, ID3D12Device* device) {
  auto db = std::make_unique<DepthBuffer>();
  if (!db->Initialize(device, width, height, *heap_manager_)) {
    return RenderGraphHandle::Invalid;
  }
  db->SetDebugName(name);

  auto index = static_cast<uint16_t>(resources_.size());
  resources_.push_back({name, RenderGraphResourceType::DepthBuffer, nullptr, db.get()});
  owned_depth_buffers_.push_back(std::move(db));
  return static_cast<RenderGraphHandle>(index);
}

RenderGraphHandle RenderGraph::ImportBackbuffer(const char* name) {
  auto index = static_cast<uint16_t>(resources_.size());
  resources_.push_back({name, RenderGraphResourceType::Backbuffer, nullptr, nullptr});
  return static_cast<RenderGraphHandle>(index);
}

uint32_t RenderGraph::GetSrvIndex(RenderGraphHandle handle) const {
  const auto& entry = GetEntry(handle);
  switch (entry.type) {
    case RenderGraphResourceType::RenderTexture:
      return entry.render_texture->GetSrvIndex();
    case RenderGraphResourceType::DepthBuffer:
      return entry.depth_buffer->GetSrvIndex();
    case RenderGraphResourceType::Backbuffer:
      assert(false && "Backbuffer has no SRV");
      return 0;
  }
  return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE RenderGraph::GetSrvGpuHandle(RenderGraphHandle handle) const {
  uint32_t index = GetSrvIndex(handle);
  return heap_manager_->GetSrvStaticAllocator().GetGpuHandle(index);
}

void RenderGraph::TransitionForRead(ID3D12GraphicsCommandList* cmd, RenderGraphHandle handle) {
  const auto& entry = GetEntry(handle);
  switch (entry.type) {
    case RenderGraphResourceType::RenderTexture:
      entry.render_texture->TransitionTo(cmd, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      break;
    case RenderGraphResourceType::DepthBuffer:
      entry.depth_buffer->TransitionTo(cmd, D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
      break;
    case RenderGraphResourceType::Backbuffer:
      assert(false && "Backbuffer cannot be transitioned for read");
      break;
  }
}

void RenderGraph::AddPass(std::unique_ptr<IRenderPass> pass) {
  passes_.push_back(std::move(pass));
  compiled_ = false;
}

void RenderGraph::MarkExternallyReferenced(RenderGraphHandle handle) {
  auto index = static_cast<uint16_t>(handle);
  assert(index < resources_.size());
  resources_[index].externally_referenced = true;
  compiled_ = false;
}

void RenderGraph::BeginFrame() {
  backbuffer_is_render_target_ = false;
  backbuffer_cleared_ = false;

  for (auto& rt : owned_render_textures_) {
    rt->ResetFrameState();
  }
  for (auto& db : owned_depth_buffers_) {
    db->ResetFrameState();
  }
}

void RenderGraph::Execute(const RenderFrameContext& frame, const FramePacket& packet) {
  if (!compiled_) Compile();

  for (uint32_t idx : execution_order_) {
    auto& pass = passes_[idx];
    if (!pass) continue;

    utils::CommandListEventGroup(frame.command_list, utils::utf8_to_wstring(pass->GetName()).c_str(), [&]() {
      ApplyPassSetup(frame.command_list, pass->GetPassSetup());
      pass->Execute(frame, packet);
    });
  }
}

void RenderGraph::ApplyPassSetup(ID3D12GraphicsCommandList* cmd, const PassSetup& setup) {
  for (auto handle : setup.resource_reads) {
    const auto& entry = GetEntry(handle);
    switch (entry.type) {
      case RenderGraphResourceType::RenderTexture:
        entry.render_texture->TransitionTo(cmd, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        break;
      case RenderGraphResourceType::DepthBuffer:
        entry.depth_buffer->TransitionTo(cmd, D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        break;
      case RenderGraphResourceType::Backbuffer:
        assert(false && "Backbuffer cannot be a shader input");
        break;
    }
  }

  std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtv_handles;
  uint32_t rt_width = 0;
  uint32_t rt_height = 0;

  for (auto handle : setup.resource_writes) {
    const auto& entry = GetEntry(handle);
    switch (entry.type) {
      case RenderGraphResourceType::Backbuffer: {
        if (!backbuffer_is_render_target_) {
          swapchain_->TransitionToRenderTarget(cmd);
          backbuffer_is_render_target_ = true;
        }
        if (!backbuffer_cleared_) {
          D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapchain_->GetCurrentRTV();
          cmd->ClearRenderTargetView(rtv, &colors::Black.x, 0, nullptr);
          backbuffer_cleared_ = true;
        }
        rtv_handles.push_back(swapchain_->GetCurrentRTV());
        if (rt_width == 0) {
          rt_width = swapchain_->GetWidth();
          rt_height = swapchain_->GetHeight();
        }
        break;
      }
      case RenderGraphResourceType::RenderTexture: {
        entry.render_texture->TransitionTo(cmd, D3D12_RESOURCE_STATE_RENDER_TARGET);
        if (entry.render_texture->NeedsClear()) {
          entry.render_texture->Clear(cmd);
        }
        rtv_handles.push_back(entry.render_texture->GetRTV());
        if (rt_width == 0) {
          rt_width = entry.render_texture->GetWidth();
          rt_height = entry.render_texture->GetHeight();
        }
        break;
      }
      case RenderGraphResourceType::DepthBuffer:
        assert(false && "DepthBuffer cannot be a color target");
        break;
    }
  }

  D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
  bool has_depth = setup.depth != RenderGraphHandle::Invalid;
  if (has_depth) {
    const auto& entry = GetEntry(setup.depth);
    assert(entry.type == RenderGraphResourceType::DepthBuffer);
    entry.depth_buffer->TransitionTo(cmd, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    if (entry.depth_buffer->NeedsClear()) {
      entry.depth_buffer->Clear(cmd);
    }
    dsv = entry.depth_buffer->GetDSV();
  }

  if (!rtv_handles.empty()) {
    cmd->OMSetRenderTargets(static_cast<UINT>(rtv_handles.size()), rtv_handles.data(), FALSE, has_depth ? &dsv : nullptr);
  }

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

void RenderGraph::FinalizeFrame(ID3D12GraphicsCommandList* cmd) {
  if (backbuffer_is_render_target_) {
    swapchain_->TransitionToPresent(cmd);
    backbuffer_is_render_target_ = false;
  }
}

void RenderGraph::Resize(ID3D12Device* device, uint32_t width, uint32_t height) {
  for (auto& rt : owned_render_textures_) {
    rt->SafeRelease(*heap_manager_);
    rt->Initialize(device, width, height, *heap_manager_);
  }
  for (auto& db : owned_depth_buffers_) {
    db->SafeRelease(*heap_manager_);
    db->Initialize(device, width, height, *heap_manager_);
  }
}

void RenderGraph::Shutdown() {
  passes_.clear();
  pass_nodes_.clear();
  execution_order_.clear();
  compiled_ = false;

  for (auto& rt : owned_render_textures_) {
    rt->SafeRelease(*heap_manager_);
  }
  owned_render_textures_.clear();

  for (auto& db : owned_depth_buffers_) {
    db->SafeRelease(*heap_manager_);
  }
  owned_depth_buffers_.clear();

  resources_.clear();
}

void RenderGraph::Compile() {
  const auto pass_count = static_cast<uint32_t>(passes_.size());
  const auto resource_count = static_cast<uint32_t>(resources_.size());

  pass_nodes_.assign(pass_count, PassNode{});
  execution_order_.clear();
  execution_order_.reserve(pass_count);

  std::vector<uint32_t> last_producer(resource_count, UINT32_MAX);

  auto link_dependency = [&](uint32_t from, uint32_t to, std::unordered_set<uint32_t>& tracked_predecessors) {
    if (tracked_predecessors.insert(from).second) {
      pass_nodes_[from].successors.push_back(to);
      pass_nodes_[to].predecessor_count++;
    }
  };

  for (auto [i, pass] : passes_ | std::views::enumerate) {
    const auto& setup = pass->GetPassSetup();
    std::unordered_set<uint32_t> tracked_predecessors;
    auto pass_index = static_cast<uint32_t>(i);

    auto process_read = [&](RenderGraphHandle handle) {
      auto ri = static_cast<uint32_t>(handle);
      if (ri < resource_count && last_producer[ri] != UINT32_MAX) {
        link_dependency(last_producer[ri], pass_index, tracked_predecessors);
      }
    };

    auto process_write = [&](RenderGraphHandle handle) {
      auto ri = static_cast<uint32_t>(handle);
      if (ri < resource_count && last_producer[ri] != UINT32_MAX) {
        link_dependency(last_producer[ri], pass_index, tracked_predecessors);
      }
      last_producer[ri] = pass_index;
    };

    std::ranges::for_each(setup.resource_reads, process_read);
    std::ranges::for_each(setup.resource_writes, process_write);

    if (setup.depth != RenderGraphHandle::Invalid) {
      process_write(setup.depth);
    }
  }

  // Kahn's algorithm with insertion-order tiebreaker (min-heap)
  auto root_passes =
    std::views::iota(0u, pass_count) | std::views::filter([&](uint32_t i) { return pass_nodes_[i].predecessor_count == 0; });
  std::priority_queue<uint32_t, std::vector<uint32_t>, std::greater<>> ready(std::greater<>{}, {root_passes.begin(), root_passes.end()});

  while (!ready.empty()) {
    uint32_t current = ready.top();
    ready.pop();
    execution_order_.push_back(current);

    for (uint32_t succ : pass_nodes_[current].successors) {
      if (--pass_nodes_[succ].predecessor_count == 0) {
        ready.push(succ);
      }
    }
  }

  if (execution_order_.size() != pass_count) {
    Logger::LogFormat(LogLevel::Error,
      LogCategory::Graphic,
      Logger::Here(),
      "Render graph has a cycle: sorted {}/{} passes",
      execution_order_.size(),
      pass_count);
  }
  assert(execution_order_.size() == pass_count && "Render graph has a cycle");

  CullDeadPasses(pass_count);
  compiled_ = true;
}

void RenderGraph::CullDeadPasses(uint32_t pass_count) {
  std::vector<std::vector<uint32_t>> predecessors(pass_count);
  for (auto [i, node] : pass_nodes_ | std::views::enumerate) {
    std::ranges::for_each(node.successors, [&](uint32_t succ) { predecessors[succ].push_back(static_cast<uint32_t>(i)); });
  }

  std::vector<bool> alive(pass_count, false);
  std::queue<uint32_t> worklist;

  auto mark_alive = [&](uint32_t i) {
    if (!alive[i]) {
      alive[i] = true;
      worklist.push(i);
    }
  };

  auto is_output_resource = [&](RenderGraphHandle handle) {
    const auto& entry = GetEntry(handle);
    return entry.type == RenderGraphResourceType::Backbuffer || entry.externally_referenced;
  };

  for (auto [i, pass] : passes_ | std::views::enumerate) {
    if (std::ranges::any_of(pass->GetPassSetup().resource_writes, is_output_resource)) {
      mark_alive(static_cast<uint32_t>(i));
    }
  }

  while (!worklist.empty()) {
    uint32_t current = worklist.front();
    worklist.pop();
    for (uint32_t pred : predecessors[current]) {
      if (!alive[pred]) {
        alive[pred] = true;
        worklist.push(pred);
      }
    }
  }

  std::erase_if(execution_order_, [&](uint32_t idx) { return !alive[idx]; });
}

const RenderGraph::ResourceEntry& RenderGraph::GetEntry(RenderGraphHandle handle) const {
  auto index = static_cast<uint16_t>(handle);
  assert(index < resources_.size());
  return resources_[index];
}
