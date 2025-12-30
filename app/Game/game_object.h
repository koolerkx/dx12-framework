#pragma once
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "Component/component.h"
#include "Component/transform_component.h"

class IScene;
class GameContext;

class GameObject {
 public:
  explicit GameObject(IScene* scene, const std::string& name = "GameObject");
  ~GameObject();

  // Lifecycle
  void Start();
  void Update(float dt);
  void FixedUpdate(float dt);
  void Render(FramePacket& packet);

  TransformComponent* GetTransform();

  IScene* GetScene() const {
    return scene_;
  }
  GameContext* GetContext() const;

  // --- Hierarchy System ---
  /// @note Pass nullptr to detach.
  void SetParent(GameObject* parent);

  GameObject* GetParent() const {
    return parent_;
  }

  const std::vector<GameObject*>& GetChildren() const {
    return children_;
  }

  // RTTI
  template <typename T, typename... Args>
  T* AddComponent(Args&&... args) {
    static_assert(std::is_base_of<IComponentBase, T>::value, "T must derive from Component");

    // Create and store
    auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
    T* ptr = component.get();
    components_.push_back(std::move(component));

    // If game already started, catch up lifecycle
    if (is_started_) {
      ptr->OnStart();
    }

    return ptr;
  }

  template <typename T, typename... Args>
  T* AddUniqueComponent(Args&&... args) {
    if (auto existing = GetComponent<T>()) {
      return existing;
    }
    return AddComponent<T>(std::forward<Args>(args)...);
  }

  template <typename T>
  T* GetComponent() const {
    ComponentTypeID target_id = ComponentType<T>::GetID();

    for (const auto& comp : components_) {
      if (comp->GetTypeID() == target_id) {
        return static_cast<T*>(comp.get());
      }
    }
    return nullptr;
  }

  const std::string& GetName() const {
    return name_;
  }

  // check if node is a descendant of possible_ancestor in order to prevent cyclic dependencies
  static bool IsDescendantOf(const GameObject* node, const GameObject* possible_ancestor);

 private:
  void AddChild(GameObject* child);
  void RemoveChild(GameObject* child);

 private:
  std::string name_;
  bool is_started_ = false;

  IScene* scene_ = nullptr;
  GameObject* parent_ = nullptr;
  std::vector<GameObject*> children_;
  std::vector<std::unique_ptr<IComponentBase>> components_;
};
