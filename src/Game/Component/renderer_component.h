#pragma once

#include "component.h"
#include "Game/game_object.h"

struct FramePacket;

class IRenderable {
 public:
  virtual ~IRenderable() = default;
  virtual void OnRender(FramePacket& packet) = 0;
  virtual bool IsRenderEnabled() const = 0;
};

namespace detail {
void RegisterRenderable(class GameObject* owner, IRenderable* renderable);
void UnregisterRenderable(class GameObject* owner, IRenderable* renderable);
}  // namespace detail

template <typename Derived>
class RendererComponent : public Component<Derived>, public IRenderable {
 public:
  using Component<Derived>::Component;

  bool IsRenderEnabled() const final {
    return this->IsEnabled() && this->IsStarted() && this->GetOwner()->IsActiveInHierarchy();
  }

  void OnInit() override {
    detail::RegisterRenderable(this->GetOwner(), this);
  }

  void OnDestroy() override {
    detail::UnregisterRenderable(this->GetOwner(), this);
  }
};
