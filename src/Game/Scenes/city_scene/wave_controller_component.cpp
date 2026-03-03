#include "Scenes/city_scene/wave_controller_component.h"

#include "Scenes/city_scene/enemy_spawn_manager_component.h"
#include "game_object.h"
#include "scene.h"

WaveControllerComponent::WaveControllerComponent(GameObject* owner, const Props& props) : BehaviorComponent(owner), props_(props) {
}

void WaveControllerComponent::OnStart() {
  auto* enemy_manager = GetOwner()->GetScene()->FindGameObject("EnemyManager");
  if (enemy_manager) {
    spawn_manager_ = enemy_manager->GetComponent<EnemySpawnManagerComponent>();
  }
}
