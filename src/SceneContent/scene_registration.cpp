#include "scene_registration.h"

#include "Game/Serialization/scene_serializer.h"
#include "Game/game_object.h"
#include "Game/scene_manager.h"
#include "Scenes/city_scene/city_scene.h"
#include "Scenes/cube_scene/cube_scene.h"
#include "Scenes/model_scene/model_scene.h"
#include "Scenes/test_scene/character_mover_component.h"
#include "Scenes/test_scene/test_scene.h"
#include "Scenes/title_scene/title_scene.h"

void RegisterUserScenes(SceneManager& manager) {
  manager.Register<TestScene>();
  manager.Register<CubeScene>();
  manager.Register<ModelScene>();
  manager.Register<CityScene>();
  manager.Register<TitleScene>();
}

void RegisterUserComponents() {
  SceneSerializer::RegisterComponentFactory("CharacterMover", [](GameObject* o) { return o->AddComponent<CharacterMover>(); });
}
