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
  virtual ~GameObject();

  void Init();
  void Start();
  void Update(float dt);
  void FixedUpdate(float dt);
  void Render(FramePacket& packet);

  bool IsStarted() const {
    return is_started_;
  }

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

    ptr->OnInit();

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

  const std::vector<std::unique_ptr<IComponentBase>>& GetComponents() const { return components_; }

  // check if node is a descendant of possible_ancestor in order to prevent cyclic dependencies
  static bool IsDescendantOf(const GameObject* node, const GameObject* possible_ancestor);

  // Mark this GameObject for destruction, remove from the scene at the end of the current frame
  void Destroy();
  bool IsPendingDestroy() const;

  void DetachFromHierarchy();

 protected:
  // Immediately after creation in Scene::CreateGameObject
  virtual void OnInit() {
  }
  // Deferred to next FlushPendingStarts (first Update or StartAllObjects)
  virtual void OnStart() {
  }
  // In destructor, before component destruction, only if started
  virtual void OnDestroy() {
  }

 private:
  void FlushPendingStarts();

  void AddChild(GameObject* child);
  void RemoveChild(GameObject* child);

 private:
  std::string name_;
  bool is_started_ = false;
  bool is_pending_destroy_ = false;

  IScene* scene_ = nullptr;
  GameObject* parent_ = nullptr;
  std::vector<GameObject*> children_;
  std::vector<std::unique_ptr<IComponentBase>> components_;
};
