/**
 * @file scene_key.h
 * @brief Hash-based scene identification with compile-time traits and runtime lookup table.
 * @note for new scene and load scene, we still use EMPTY and BLANK now, which may loading incorrect scene
 */
#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "Framework/Core/utils.h"

using SceneKey = uint32_t;

constexpr SceneKey MakeSceneKey(std::string_view name) {
  return utils::HashString(name);
}

template <typename Scene>
struct SceneKeyTrait;

namespace DefaultScenes {
inline constexpr SceneKey EMPTY = MakeSceneKey("empty");
inline constexpr SceneKey BLANK = MakeSceneKey("blank");
}  // namespace DefaultScenes

struct SceneKeyEntry {
  SceneKey key;
  std::string_view name;
};

class SceneKeyTable {
 public:
  static SceneKeyTable& Instance();

  void Register(SceneKey key, std::string_view name);
  std::string_view FindName(SceneKey key) const;
  const std::vector<SceneKeyEntry>& GetAll() const;

 private:
  SceneKeyTable() = default;
};
