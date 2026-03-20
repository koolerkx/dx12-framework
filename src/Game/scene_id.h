#pragma once
#include <cstdint>

enum class SceneId : uint8_t { TITLE_SCENE, TEST_SCENE, CUBE_SCENE, EMPTY_SCENE, BLANK_SCENE, MODEL_SCENE, CITY_SCENE };

constexpr const char* SceneIdLabel(SceneId id) {
  switch (id) {
    case SceneId::TITLE_SCENE:
      return "Title";
    case SceneId::TEST_SCENE:
      return "Test";
    case SceneId::CUBE_SCENE:
      return "Cube";
    case SceneId::EMPTY_SCENE:
      return "Empty";
    case SceneId::BLANK_SCENE:
      return "Blank";
    case SceneId::MODEL_SCENE:
      return "Model";
    case SceneId::CITY_SCENE:
      return "City";
    default:
      return "Unknown";
  }
}
