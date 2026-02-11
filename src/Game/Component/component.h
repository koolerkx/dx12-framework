#pragma once

#include <typeinfo>

#include "Graphic/Frame/frame_packet.h"

// Define a unique ID type for components
using ComponentTypeID = uint32_t;
inline ComponentTypeID GetNextComponentID() {
  static ComponentTypeID count = 0;
  return count++;
}

// Templated structure to assign a unique ID per class type at compile/runtime start
template <typename T>
struct ComponentType {
  static ComponentTypeID GetID() {
    static ComponentTypeID id = GetNextComponentID();
    return id;
  }
};

class GameObject;
class GameContext;

class IComponentBase {
 public:
  explicit IComponentBase(GameObject* owner) : owner_(owner) {
  }
  virtual ~IComponentBase() = default;

  // Immediately on AddComponent, before Start
  virtual void OnInit() {
  }
  // Deferred to next FlushPendingStarts (first Update or StartAllObjects)
  virtual void OnStart() {
  }
  // Manual only — never auto-called by the system
  virtual void OnReset() {
  }
  // Every frame, gated by enabled and started
  virtual void OnUpdate(float /*dt*/) {
  }
  // Every fixed timestep, gated by enabled and started
  virtual void OnFixedUpdate(float /*dt*/) {
  }
  // Every frame render pass, gated by enabled and started
  virtual void OnRender(FramePacket& /*packet*/) {
  }
  // In ~GameObject, after GameObject::OnDestroy, only if started
  virtual void OnDestroy() {
  }
  // When owning GameObject's parent changes via SetParent
  virtual void OnParentChanged() {
  }

  virtual ComponentTypeID GetTypeID() const = 0;
  virtual const char* GetTypeName() const {
    return "Unknown";
  }

  GameObject* GetOwner() const {
    return owner_;
  }
  GameContext* GetContext() const;

  bool IsEnabled() const {
    return enabled_;
  }
  void SetEnabled(bool enabled) {
    enabled_ = enabled;
  }
  bool IsStarted() const {
    return is_started_;
  }

 protected:
  GameObject* owner_;

 private:
  bool enabled_ = true;
  bool is_started_ = false;

  friend class GameObject;
};

// CTRP for Component GetTypeID
template <typename Derived>
class Component : public IComponentBase {
 public:
  using IComponentBase::IComponentBase;

  ComponentTypeID GetTypeID() const final override {
    return ComponentType<Derived>::GetID();
  }

  const char* GetTypeName() const override {
    const std::type_info& info = typeid(Derived);
    return info.name();
  }
};
