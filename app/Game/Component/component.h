#pragma once

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

  virtual void OnStart() {
  }
  virtual void OnUpdate(float /*dt*/) {
  }
  virtual void OnFixedUpdate(float /*dt*/) {
  }
  virtual void OnRender(FramePacket& /*packet*/) {
  }
  virtual void OnParentChanged() {
  }

  virtual ComponentTypeID GetTypeID() const = 0;

  GameObject* GetOwner() const {
    return owner_;
  }

  GameContext* GetContext() const;

 protected:
  GameObject* owner_;
};

// CTRP for Component GetTypeID
template <typename Derived>
class Component : public IComponentBase {
 public:
  using IComponentBase::IComponentBase;

  // Automatically implemented - user never writes this!
  ComponentTypeID GetTypeID() const final override {
    return ComponentType<Derived>::GetID();
  }
};
