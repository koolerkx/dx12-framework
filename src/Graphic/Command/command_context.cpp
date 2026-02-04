#include "command_context.h"

#include "Framework/Logging/logger.h"

namespace gfx {

std::unique_ptr<CommandContext> CommandContext::Create(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type) {
  auto context = std::unique_ptr<CommandContext>(new CommandContext());
  if (!context->Initialize(device, type)) {
    return nullptr;
  }
  return context;
}

bool CommandContext::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type) {
  if (!CreateCommandQueue(device, type)) {
    return false;
  }

  if (!CreateCommandAllocators(device, type)) {
    return false;
  }

  if (!CreateCommandList(device, type)) {
    return false;
  }

  Logger::LogFormat(LogLevel::Info, LogCategory::Graphic, Logger::Here(), "CommandContext initialized");
  return true;
}

bool CommandContext::CreateCommandQueue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type) {
  D3D12_COMMAND_QUEUE_DESC desc{};
  desc.Type = type;
  desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
  desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  desc.NodeMask = 0;

  HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue_));
  if (FAILED(hr) || !queue_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create command queue: 0x{:08X}", hr);
    return false;
  }

  queue_->SetName(L"CommandContext_Queue");
  return true;
}

bool CommandContext::CreateCommandAllocators(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type) {
  HRESULT hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&utility_allocator_));
  if (FAILED(hr) || !utility_allocator_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create utility allocator: 0x{:08X}", hr);
    return false;
  }
  utility_allocator_->SetName(L"CommandContext_UtilityAllocator");

  for (uint32_t i = 0; i < kMaxFramesInFlight; ++i) {
    hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&frame_allocators_[i]));
    if (FAILED(hr) || !frame_allocators_[i]) {
      Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create frame allocator {}: 0x{:08X}", i, hr);
      return false;
    }

    std::wstring name = L"CommandContext_FrameAllocator" + std::to_wstring(i);
    frame_allocators_[i]->SetName(name.c_str());
  }

  return true;
}

bool CommandContext::CreateCommandList(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type) {
  HRESULT hr = device->CreateCommandList(0, type, utility_allocator_.Get(), nullptr, IID_PPV_ARGS(&command_list_));
  if (FAILED(hr) || !command_list_) {
    Logger::LogFormat(LogLevel::Fatal, LogCategory::Graphic, Logger::Here(), "Failed to create command list: 0x{:08X}", hr);
    return false;
  }

  command_list_->SetName(L"CommandContext_CommandList");
  command_list_->Close();
  return true;
}

void CommandContext::BeginFrame(uint32_t frame_index) {
  current_frame_index_ = frame_index;

  auto* allocator = frame_allocators_[frame_index].Get();
  allocator->Reset();
  command_list_->Reset(allocator, nullptr);
}

void CommandContext::Execute() {
  command_list_->Close();
  ID3D12CommandList* lists[] = {command_list_.Get()};
  queue_->ExecuteCommandLists(1, lists);
}

void CommandContext::ExecuteSync(ID3D12Fence* fence, std::function<void(ID3D12GraphicsCommandList*)> recorder) {
  utility_allocator_->Reset();
  command_list_->Reset(utility_allocator_.Get(), nullptr);

  recorder(command_list_.Get());

  command_list_->Close();
  ID3D12CommandList* lists[] = {command_list_.Get()};
  queue_->ExecuteCommandLists(1, lists);

  if (fence) {
    uint64_t current_value = fence->GetCompletedValue();
    uint64_t signal_value = current_value + 1;

    queue_->Signal(fence, signal_value);

    HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (event) {
      fence->SetEventOnCompletion(signal_value, event);
      WaitForSingleObject(event, INFINITE);
      CloseHandle(event);
    }
  }
}

}  // namespace gfx
