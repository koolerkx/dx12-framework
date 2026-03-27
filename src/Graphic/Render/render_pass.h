#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Frame/render_frame_context.h"
#include "Framework/Render/frame_packet.h"

enum class RenderGraphHandle : uint16_t { Invalid = 0xFFFF };

struct PassSetup {
  std::vector<RenderGraphHandle> resource_writes;
  RenderGraphHandle depth = RenderGraphHandle::Invalid;
  std::vector<RenderGraphHandle> resource_reads;
};

class IRenderPass {
 public:
  virtual ~IRenderPass() = default;

  virtual void Execute(const RenderFrameContext& frame, const FramePacket& packet) = 0;
  virtual const char* GetName() const = 0;

  virtual const wchar_t* GetWideName() const {
    if (cached_wide_name_.empty()) {
      const char* n = GetName();
      cached_wide_name_.assign(n, n + std::char_traits<char>::length(n));
    }
    return cached_wide_name_.c_str();
  }

  const PassSetup& GetPassSetup() const {
    return setup_;
  }

  void SetGroupName(const char* name) {
    group_name_ = name;
    wide_group_name_.assign(name, name + std::char_traits<char>::length(name));
  }

  const char* GetGroupName() const {
    return group_name_;
  }

  const wchar_t* GetWideGroupName() const {
    return wide_group_name_.c_str();
  }

 protected:
  PassSetup setup_;
  const char* group_name_ = nullptr;
  std::wstring wide_group_name_;
  mutable std::wstring cached_wide_name_;
};
