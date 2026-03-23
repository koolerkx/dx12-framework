#include "scene_key.h"

#include <unordered_map>

namespace {
std::unordered_map<SceneKey, std::string_view>& NameMap() {
  static std::unordered_map<SceneKey, std::string_view> map;
  return map;
}

std::vector<SceneKeyEntry>& Entries() {
  static std::vector<SceneKeyEntry> entries;
  return entries;
}
}  // namespace

SceneKeyTable& SceneKeyTable::Instance() {
  static SceneKeyTable instance;
  return instance;
}

void SceneKeyTable::Register(SceneKey key, std::string_view name) {
  auto& map = NameMap();
  if (map.contains(key)) return;
  map[key] = name;
  Entries().push_back({key, name});
}

std::string_view SceneKeyTable::FindName(SceneKey key) const {
  auto& map = NameMap();
  auto it = map.find(key);
  return it != map.end() ? it->second : "";
}

const std::vector<SceneKeyEntry>& SceneKeyTable::GetAll() const {
  return Entries();
}
