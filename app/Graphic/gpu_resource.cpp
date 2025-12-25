#include "gpu_resource.h"

#include "Core/utils.h"

void GpuResource::TransitionTo(ID3D12GraphicsCommandList* command_list, D3D12_RESOURCE_STATES new_state, UINT subresource) {
  if (!resource_) {
    return;
  }

  if (current_state_ == new_state) {
    return;
  }

  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = resource_.Get();
  barrier.Transition.Subresource = subresource;
  barrier.Transition.StateBefore = current_state_;
  barrier.Transition.StateAfter = new_state;

  command_list->ResourceBarrier(1, &barrier);
  current_state_ = new_state;
}

void GpuResource::SetDebugName(const std::string& name) {
  if (resource_) {
    resource_->SetName(utils::utf8_to_wstring(name).c_str());
  }
  debug_name = name;
}

void GpuResource::SetDebugName(const std::wstring& name) {
  if (resource_) {
    resource_->SetName(name.c_str());
  }
  debug_name = utils::wstring_to_utf8(name);
}
