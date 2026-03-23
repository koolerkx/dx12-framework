/**
 * @file scene_key.h
 * @brief Hash-based scene identification using compile-time string hashing.
 */
#pragma once

#include <cstdint>

#include "Framework/Core/utils.h"

using SceneKey = uint32_t;

constexpr SceneKey MakeSceneKey(std::string_view name) {
  return utils::HashString(name);
}

namespace DefaultScenes {
inline constexpr SceneKey EMPTY = MakeSceneKey("empty");
inline constexpr SceneKey BLANK = MakeSceneKey("blank");
}  // namespace DefaultScenes

namespace UserScenes {
inline constexpr SceneKey TITLE = MakeSceneKey("title");
inline constexpr SceneKey TEST = MakeSceneKey("test");
inline constexpr SceneKey CUBE = MakeSceneKey("cube");
inline constexpr SceneKey MODEL = MakeSceneKey("model");
inline constexpr SceneKey CITY = MakeSceneKey("city");
}  // namespace UserScenes
