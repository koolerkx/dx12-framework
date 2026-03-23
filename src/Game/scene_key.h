/**
 * @file scene_key.h
 * @brief String-based scene identification replacing the former SceneId enum.
 */
#pragma once
#include <string>

using SceneKey = std::string;

namespace DefaultScenes {
inline const SceneKey EMPTY = "empty";
inline const SceneKey BLANK = "blank";
}  // namespace DefaultScenes

// FIXME: Move to SceneContent in Phase 2
namespace UserScenes {
inline const SceneKey TITLE = "title";
inline const SceneKey TEST = "test";
inline const SceneKey CUBE = "cube";
inline const SceneKey MODEL = "model";
inline const SceneKey CITY = "city";
}  // namespace UserScenes
