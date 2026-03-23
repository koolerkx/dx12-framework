#pragma once

#include <memory>

#include "System/system.h"

struct ModelData;
class GameObject;
class IScene;

namespace Math {
struct Vector3;
}

class TurretSystem : public ISystem {
 public:
  explicit TurretSystem(IScene* scene);
  void Update(float dt) override;

 private:
  void SpawnProjectile(const Math::Vector3& origin, GameObject* target, float damage);

  IScene* scene_ = nullptr;
  std::shared_ptr<ModelData> projectile_model_;
};
