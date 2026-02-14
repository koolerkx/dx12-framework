#pragma once

#include <d3d12.h>

#include <array>
#include <cstdint>
#include <functional>
#include <memory>

#include "Core/types.h"

namespace gfx {

class CommandContext {
 public:
  static constexpr uint32_t kMaxFramesInFlight = 2;

  [[nodiscard]] static std::unique_ptr<CommandContext> Create(
    ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

  CommandContext(const CommandContext&) = delete;
  CommandContext& operator=(const CommandContext&) = delete;
  ~CommandContext() = default;

  ID3D12CommandQueue* GetQueue() const {
    return queue_.Get();
  }

  ID3D12GraphicsCommandList* GetCommandList() const {
    return command_list_.Get();
  }

  void BeginFrame(uint32_t frame_index);

  void Execute();

  void Submit(std::function<void(ID3D12GraphicsCommandList*)> recorder);

 private:
  CommandContext() = default;
  bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);

  bool CreateCommandQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
  bool CreateCommandAllocators(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
  bool CreateCommandList(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);

  ComPtr<ID3D12CommandQueue> queue_;
  ComPtr<ID3D12GraphicsCommandList> command_list_;
  std::array<ComPtr<ID3D12CommandAllocator>, kMaxFramesInFlight> frame_allocators_;
  ComPtr<ID3D12CommandAllocator> utility_allocator_;

  uint32_t current_frame_index_ = 0;
};

}  // namespace gfx
